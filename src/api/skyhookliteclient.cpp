/*
 * Copyright 2014-present Skyhook Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "api/skyhookliteclient.h"

#include "spi/WifiAdapter.h"
#include "spi/CellAdapter.h"
#include "spi/GPSAdapter.h"
#include "spi/XmlHttpRequest.h"
#include "spi/Concurrent.h"
#include "spi/Logger.h"
#include "spi/Time.h"
#include "spi/DOM.h"
#include "spi/XmlParser.h"
#include "spi/SystemInformation.h"

#include "Protocol.h"
#include "XmlUtils.h"
#include "version.h"

#include <md4.h>
#include <memory>
#include <vector>
#include <algorithm>

using namespace WPS::SPI;
using namespace WPS::API;

static const unsigned TIMEOUT = 20 * 1000;

class WifiWrapper
    : public WifiAdapter::Listener
{
public:

	WifiWrapper()
	    : _event(Event::newInstance())
        , _rc(SPI_ERROR_NOT_READY)
	{}

	ErrorCode open()
	{
		_wifi.reset(WifiAdapter::newInstance());
		if (_wifi.get() == NULL)
            return SPI_ERROR;
        _wifi->setListener(this);
		return _wifi->open();
	}

    void close()
    {
        _wifi.reset();
    }

	ErrorCode scan(unsigned long timeout,
                   std::vector<ScannedAccessPoint>& scannedAPs)
	{
        if (_wifi.get() == NULL)
            return SPI_ERROR;

        _event->clear();
        _wifi->startScan();

        if (_event->wait(timeout) != 0)
            return SPI_ERROR;

        if (_rc != SPI_OK)
            return _rc;

        scannedAPs = _scan;
        return SPI_OK;
	}

    std::string getHardwareMAC()
    {
        MAC mac;
        if (_wifi->getHardwareMAC(mac) != SPI_OK)
            return "";
        if (mac.toLong() == 0)
            return "";
        return mac.toString();
    }

private:

    void onScanCompleted(const std::vector<ScannedAccessPoint>& scannedAPs)
    {
        _rc = SPI_OK;
    	_scan = scannedAPs;
        _event->signal();
    }

    void onScanFailed(ErrorCode code)
    {
        _rc = code;
        _scan.clear();
        _event->signal();
    }

private:

	std::auto_ptr<WifiAdapter> _wifi;
    std::auto_ptr<Event> _event;
    ErrorCode _rc;
    std::vector<ScannedAccessPoint> _scan;
};

class GpsWrapper
    : public GPSAdapter::Listener
{
public:

	GpsWrapper()
        : _mutex(Mutex::newInstance())
	{}

	ErrorCode open()
	{
		_gps.reset(GPSAdapter::newInstance());
		if (_gps.get() == NULL)
            return SPI_ERROR;
        _gps->setListener(this);
		return _gps->open();
	}

    void close()
    {
        _gps.reset();
    }

    const std::vector<GPSData::Fix>& getFixes()
    {
        Guard guard(_mutex.get());
        return _fixes;
    }

private:

    void onGpsData(const GPSData& gpsData)
    {
        Guard guard(_mutex.get());

        if (gpsData.fix.get())
        {
            const GPSData::Fix newFix = *gpsData.fix;

            if (_fixes.empty() || _fixes.back().gpsTime != newFix.gpsTime)
                _fixes.push_back(newFix);
        }
    }

    void onGpsError(ErrorCode)
    {}

private:

    std::auto_ptr<Mutex> _mutex;
	std::auto_ptr<GPSAdapter> _gps;
    std::vector<GPSData::Fix> _fixes;
};

class CellWrapper
    : public CellAdapter::Listener
{
public:

	CellWrapper()
        : _mutex(Mutex::newInstance())
	{}

	ErrorCode open()
	{
		_cellAdapter.reset(CellAdapter::newInstance());
		if (_cellAdapter.get() == NULL)
            return SPI_ERROR;
        _cellAdapter->setListener(this);
		return _cellAdapter->open();
	}

    const std::vector<ScannedCellTower>& getScannedCells()
    {
        Guard guard(_mutex.get());
        return _scannedCells;
    }

    std::string getIMEI()
    {
        Guard guard(_mutex.get());
        if (_cellAdapter.get() == NULL)
            return "";
        std::string imei;
        if (_cellAdapter->getIMEI(imei) != SPI_OK)
            return "";
        return imei;
    }

private:

    void onCellChanged(const std::vector<ScannedCellTower>& scannedCells)
    {
        Guard guard(_mutex.get());
        _scannedCells = scannedCells;
    }

    void onCellError(ErrorCode)
    {}

private:

    std::auto_ptr<Mutex> _mutex;
	std::auto_ptr<CellAdapter> _cellAdapter;
    std::vector<ScannedCellTower> _scannedCells;
};

std::string
md4(const std::string& input)
{
    unsigned char digest[16];

    WPS_MD4_CTX ctx;
    WPS_MD4Init(&ctx);
    WPS_MD4Update(&ctx, (unsigned char*) input.c_str(), (unsigned int) input.length());
    WPS_MD4Final(digest, &ctx);

    char buf[64];
    WPS::SPI::snprintf(buf,
                       sizeof(buf),
                       "%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x",
                       digest[0],
                       digest[1],
                       digest[2],
                       digest[3],
                       digest[4],
                       digest[5],
                       digest[6],
                       digest[7],
                       digest[8],
                       digest[9],
                       digest[10],
                       digest[11],
                       digest[12],
                       digest[13],
                       digest[14],
                       digest[15]);
    return buf;
}

inline bool
isNotValidForMeta(char c)
{
    // ASCII non-printable characters or semicolon
    // (since it acts as a delimiter).
    return c < 32 || c > 126 || c == ';';
}

inline bool
isValidForMeta(const std::string& s)
{
    return std::find_if(s.begin(), s.end(), isNotValidForMeta) == s.end();
}

inline std::string
validateForMeta(const std::string& s)
{
    return isValidForMeta(s) ? s : "";
}

static std::string
getDeviceUsername(WifiWrapper& wifi, CellWrapper& cell)
{
    const std::string mac = wifi.getHardwareMAC();
    if (! mac.empty())
        return md4(mac);

    const std::string imei = cell.getIMEI();
    if (! imei.empty())
        return md4(imei);

    return "";
}

static std::string
getMetaString()
{
    std::string meta;
    meta.reserve(64);

    SystemInformation::OSInfo osInfo;
    SystemInformation::DeviceInfo deviceInfo;

    std::auto_ptr<SystemInformation> sysInfo(SystemInformation::newInstance());
    if (sysInfo.get())
    {
        sysInfo->getOSInfo(osInfo);
        sysInfo->getDeviceInfo(deviceInfo);
    }

    meta.append("1;shlc;")
        .append(SHLC_VERSION)
        .append(";")
        .append(validateForMeta(osInfo.type))
        .append(";")
        .append(validateForMeta(osInfo.version))
        .append(";")
        .append(validateForMeta(deviceInfo.manufacturer))
        .append(";")
        .append(validateForMeta(deviceInfo.model));

    return meta;
}

static SHLC_ReturnCode
getLocation(const char* key,
            const char* username,
            const Scan& scan,
            SHLC_Location** location)
{
    std::string rq;
    Protocol::locationRQ(key, username, scan, rq);

    std::auto_ptr<XmlHttpRequest> xhr(XmlHttpRequest::newInstance());

    xhr->open(XmlHttpRequest::HTTP_POST, "https://api.skyhookwireless.com/wps2/location");
    xhr->setRequestHeader("Content-Type", "text/xml");
    xhr->setRequestHeader("Skyhook-Meta", getMetaString());

    ErrorCode code = xhr->send(rq);
    if (code != SPI_OK)
        return SHLC_ERROR_SERVER_UNAVAILABLE;

    switch (xhr->getStatusCode())
    {
        case XmlHttpRequest::OK:
            break;
        case XmlHttpRequest::UNAUTHORIZED:
            return SHLC_ERROR_UNAUTHORIZED;
        default:
            return SHLC_ERROR_SERVER_UNAVAILABLE;
    }

    const std::string rs = xhr->getResponseData();

    std::auto_ptr<XmlParser> parser(XmlParser::newInstance());
    std::auto_ptr<DOMDocument> doc(parser->parse(rs.data(), rs.size()));
    if (! doc.get())
        return SHLC_ERROR_LOCATION_CANNOT_BE_DETERMINED;

    std::vector<LiteLocation> locations;
    if (! Protocol::parseLocationRS(doc.get(), 0, locations))
        return SHLC_ERROR_LOCATION_CANNOT_BE_DETERMINED;

    if (locations.empty())
        return SHLC_ERROR_LOCATION_CANNOT_BE_DETERMINED;

    *location = locations.front();
    return SHLC_OK;
}

const char*
SHLC_version()
{
	return SHLC_VERSION;
}

void*
SHLC_init()
{
    /*
     * This is a placeholder to allow the API to easily add global data
     */
    return (void*) 0x1;
}

void
SHLC_deinit(const void* handle)
{}

SHLC_ReturnCode
SHLC_location(const void* handle,
			  const char* key,
              SHLC_Location** location)
{
	WifiWrapper wifi;
    CellWrapper cell;
    GpsWrapper gps;
    Scan scan;

    gps.open();
    cell.open();

	if (wifi.open() != SPI_OK)
		return SHLC_ERROR_RADIO_NOT_AVAILABLE;

    const std::string username = getDeviceUsername(wifi, cell);
    if (username.empty())
        return SHLC_ERROR_UNAUTHORIZED;

    if (wifi.scan(TIMEOUT, scan.aps) != SPI_OK)
        return SHLC_ERROR_RADIO_NOT_AVAILABLE;

    /*
     * Wi-Fi scan completed
     */
    scan.gps = gps.getFixes();
    scan.cells = cell.getScannedCells();

    if (scan.aps.empty() && scan.cells.empty() && scan.gps.empty())
        return SHLC_ERROR_NO_BEACONS_IN_RANGE;

    /*
     * Determine location remotely
     */
    return getLocation(key, username.c_str(), scan, location);
}

void
SHLC_free_location(const void* handle,
                   SHLC_Location* location)
{
    LiteLocation::free_location(location);
}
