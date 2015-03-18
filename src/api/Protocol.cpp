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

#include "Protocol.h"
#include "XmlUtils.h"

#include "spi/DOM.h"
#include "spi/Time.h"
#include "spi/XmlParser.h"
#include "spi/StdLibC.h"

#include <memory>
#include <cmath>
#include <string.h>

#include "spi/Assert.h"

namespace {

static const char* VERSION = "2.23";

static const size_t AUTH_STR_SIZE = 128 + 32;
static const size_t ADDR_LOOKUP_STR_SIZE = 64;

static const size_t AP_STR_SIZE = 128;
static const size_t SSID_STR_SIZE = 45;
static const size_t CELL_STR_SIZE = 128;  // about 83 for gsm-tower and 106 for cdma-tower in worst case
static const size_t GPS_STR_SIZE = 256;

static inline size_t
sizeNumSsids(size_t size)
{
    return size * SSID_STR_SIZE;
}

static inline size_t
sizeNumAPs(size_t size, bool includeSsid)
{
    return size * AP_STR_SIZE
    + (includeSsid ? sizeNumSsids(size) : 0);
}

static inline size_t
sizeNumCells(size_t size)
{
    return size * CELL_STR_SIZE;
}

static inline size_t
sizeNumGps(size_t size)
{
    return size * GPS_STR_SIZE;
}

static inline size_t
size(const std::vector<WPS::SPI::ScannedAccessPoint>& aps, bool includeSsid)
{
    return sizeNumAPs(aps.size(), includeSsid);
}

static inline size_t
size(const std::vector<WPS::SPI::ScannedCellTower>& cells)
{
    return sizeNumCells(cells.size());
}

static inline size_t
size(const std::vector<WPS::SPI::GPSData::Fix>& fixes)
{
    return sizeNumGps(fixes.size());
}

static inline size_t
size(const WPS::API::Scan& scan, bool includeSsid)
{
    return size(scan.aps, includeSsid)
         + size(scan.cells)
         + size(scan.gps);
}

}  // anonymous namespace


namespace WPS {
namespace API {

using SPI::XmlParser;
using SPI::DOMNode;
using SPI::DOMNodeList;
using SPI::DOMDocument;
using SPI::GPSData;
using SPI::ScannedAccessPoint;
using SPI::CellTower;
using SPI::ScannedCellTower;
using SPI::Timer;
using SPI::Time;

static const char* namespaceURI = "http://skyhookwireless.com/wps/2005";

/**********************************************************************/
/*                                                                    */
/* Requests                                                           */
/*                                                                    */
/**********************************************************************/

inline void
reserve(std::string& xml, size_t n)
{
    size_t free = xml.capacity() - xml.size();
    if (free < n)
    {
        // Force the capacity back to zero since some string implementations
        // will reserve more than asked for (double the old capacity).
        if (xml.size() == 0)
            std::string().swap(xml);

        xml.reserve(xml.size() + n);
    }
}

template <size_t size>
inline std::string&
operator<<(std::string& xml, const char (&rhs)[size])
{
    return xml.append(rhs, size - 1);
}

inline std::string&
operator<<(std::string& xml, const char* rhs)
{
    return xml.append(rhs);
}

inline std::string&
operator<<(std::string& xml, const std::string& rhs)
{
    return xml.append(rhs);
}

inline std::string&
operator<<(std::string& xml, int i)
{
    // save string memory allocation by not using itoa
    char s[32];
    int n = SPI::snprintf(s, sizeof s, "%d", i);
    return xml.append(s, n);
}

inline std::string&
operator<<(std::string& xml, long l)
{
    // save string memory allocation by not using ltoa
    char s[32];
    int n = SPI::snprintf(s, sizeof s, "%ld", l);
    return xml.append(s, n);
}

inline std::string&
operator<<(std::string& xml, unsigned long l)
{
    // save string memory allocation by not using ltoa
    char s[32];
    int n = SPI::snprintf(s, sizeof s, "%lu", l);
    return xml.append(s, n);
}

inline std::string&
operator<<(std::string& xml, double d)
{
    char s[32];
    int n = SPI::snprintf(s, sizeof s, "%.6f", d);
    return xml.append(s, n);
}

inline std::string&
operator<<(std::string& xml, bool b)
{
    return xml.append(b ? "true" : "false");
}

struct xVersion
{
    xVersion(const char* version = VERSION)
        : version(version)
    {}

    friend std::string&
    operator<<(std::string& xml, const xVersion& rhs)
    {
        return xml << "xmlns='"
                   << namespaceURI
                   << "' version='"
                   << rhs.version
                   << "'";
    }

    const char* version;
};

struct xAuthentication
{
    xAuthentication(const char* key, const char* username)
        : key(key)
        , username(username)
    {}

    friend std::string&
    operator<<(std::string& xml, const xAuthentication& rhs)
    {
        reserve(xml, 128 + strlen(rhs.key));
        return xml << "<authentication version='2.2'><key key='"
                   << rhs.key
                   << "' username='"
                   << rhs.username
                   << "'/></authentication>";
    }

    const char* key;
    const char* username;
};

struct xAccessPoints
{
    xAccessPoints(const Timer& now,
                  const std::vector<ScannedAccessPoint>& scannedAPs,
                  bool includeSsid)
        : now(now)
        , scannedAPs(scannedAPs)
        , includeSsid(includeSsid)
    {}

    friend std::string&
    operator<<(std::string& xml, const xAccessPoints& rhs)
    {
        // NOTE: rhs.scannedAPs shouldn't have duplicates

        reserve(xml, size(rhs.scannedAPs, rhs.includeSsid));

        for (std::vector<ScannedAccessPoint>::const_iterator i = rhs.scannedAPs.begin();
             i != rhs.scannedAPs.end();
             ++i)
        {
            xml << "<access-point><mac>"
                << i->getMAC().toString()
                << "</mac>";

            // NOTE: SSID attribute must be 1-32 characters. For now we do not
            //       send SSID for hidden APs.
            const std::vector<unsigned char>& ssid = i->getSsid();
            if (rhs.includeSsid && ! ssid.empty() && xmlUtf8Test(ssid))
                xml << "<ssid>"
                    << xmlEscape(std::string(ssid.begin(), ssid.end()))
                    << "</ssid>";

            xml << "<signal-strength>"
                << i->getRSSI()
                << "</signal-strength>";

            const long age = rhs.now.delta(i->getTimestamp());
            if (age > 0)
            {
                xml << "<age>"
                    << age
                    << "</age>";
            }

            xml << "</access-point>";
        }

        return xml;
    }

    const Timer& now;
    const std::vector<ScannedAccessPoint>& scannedAPs;
    const bool includeSsid;
};

struct xCellTowers
{
    xCellTowers(const Timer& now,
                const std::vector<ScannedCellTower>& scannedCells)
        : now(now), scannedCells(scannedCells)
    {}

    friend std::string&
    operator<<(std::string& xml, const xCellTowers& rhs)
    {
        // TODO: we should assert that rhs.scannedCells doesn't have duplicates

        reserve(xml, size(rhs.scannedCells));

        for (std::vector<ScannedCellTower>::const_iterator i = rhs.scannedCells.begin();
             i != rhs.scannedCells.end();
             ++i)
        {
            CellTower::CellTowerType type = i->getCell().getType();
            if (type == CellTower::GSM || type == CellTower::UMTS)
            {
                xml << (type == CellTower::GSM
                            ? "<gsm-tower>"
                            : "<umts-tower>");
                xml << "<mcc>"
                    << i->getCell().getMcc()
                    << "</mcc><mnc>"
                    << i->getCell().getMnc()
                    << "</mnc>"
                    << "<lac>"
                    << i->getCell().getLac()
                    << "</lac><ci>"
                    << i->getCell().getCi()
                    << "</ci>";
            }
            else
            {
                assert(type == CellTower::LTE);
                xml << "<lte-tower><mcc>"
                    << i->getCell().getMcc()
                    << "</mcc><mnc>"
                    << i->getCell().getMnc()
                    << "</mnc><eucid>"
                    << i->getCell().getCi()
                    << "</eucid>";
            }

            xml << "<rssi>"
                << i->getRssi()
                << "</rssi>";

            if (i->getTimingAdvance() != 0)
            {
                xml << "<timing-advance>"
                    << i->getTimingAdvance()
                    << "</timing-advance>";
            }

            const long age = rhs.now.delta(i->getTimestamp());
            if (age > 0)
            {
                xml << "<age>"
                    << age
                    << "</age>";
            }

            if (type == CellTower::GSM)
                xml << "</gsm-tower>";
            else if (type == CellTower::UMTS)
                xml << "</umts-tower>";
            else
                xml << "</lte-tower>";
        }

        return xml;
    }

    const Timer& now;
    const std::vector<ScannedCellTower>& scannedCells;
};

struct xGPSLocations
{
    xGPSLocations(const Timer& now,
                  const std::vector<GPSData::Fix>& fixes)
        : now(now), fixes(fixes)
    {}

    friend std::string&
    operator<<(std::string& xml, const xGPSLocations& rhs)
    {
        // TODO: we should assert that rhs.fixes doesn't have duplicates

        reserve(xml, size(rhs.fixes));

        for (std::vector<GPSData::Fix>::const_iterator fix = rhs.fixes.begin();
             fix != rhs.fixes.end();
             ++fix)
        {
            xml << "<gps-location fix='"
                << fix->quality
                << "' nsat='"
                << static_cast<int>(fix->svInFix)
                << "'><latitude>"
                << fix->latitude
                << "</latitude><longitude>"
                << fix->longitude
                << "</longitude>";

            if (fix->hasHpe())
            {
                xml << "<hpe>"
                    << static_cast<int>(fix->hpe)
                    << "</hpe>";
            }

            if (fix->hasAltitude())
            {
                xml << "<altitude>"
                    << fix->altitude
                    << "</altitude>";
            }

            if (fix->hasHeight())
            {
                xml << "<height>"
                    << fix->height
                    << "</height>";
            }

            if (fix->hasSpeed())
            {
                xml << "<speed>"
                    << fix->speed
                    << "</speed>";
            }

            if (fix->hasBearing())
            {
                xml << "<bearing>"
                    << fix->bearing
                    << "</bearing>";
            }

            const long age = rhs.now.delta(fix->localTime);
            if (age >= 0)
                xml << "<age>"
                    << age
                    << "</age>";

            xml << "</gps-location>";
        }

        return xml;
    }

    const Timer& now;
    const std::vector<GPSData::Fix>& fixes;
};

struct xLocationCommon
{
    xLocationCommon(const char* key,
                    const char* username,
                    const Scan& scan,
                    const bool includeSsid)
        : key(key)
        , username(username)
        , scan(scan)
        , includeSsid(includeSsid)
    {}

    friend std::string&
    operator<<(std::string& xml, const xLocationCommon& rhs)
    {
        Timer now;
        return xml << xAuthentication(rhs.key, rhs.username)
                   << xAccessPoints(now, rhs.scan.aps, rhs.includeSsid)
                   << xCellTowers(now, rhs.scan.cells)
                   << xGPSLocations(now, rhs.scan.gps);
    }

    const char* const key;
    const char* const username;
    const Scan& scan;
    const bool includeSsid;
};

/**********************************************************************/
/* locationRQ                                                         */
/**********************************************************************/

/*static*/
void
Protocol::locationRQ(const char* key,
                     const char* username,
                     const Scan& scan,
                     std::string& out,
                     bool includeSsid)
{
    out.clear();
    reserve(out, 256 + ADDR_LOOKUP_STR_SIZE + AUTH_STR_SIZE + size(scan, includeSsid));
    out << "<LocationRQ "
        << xVersion();

    out << ">"
        << xLocationCommon(key, username, scan, includeSsid)
        << "</LocationRQ>";
}

/**********************************************************************/
/*                                                                    */
/* Responses                                                          */
/*                                                                    */
/**********************************************************************/

static DOMNode*
selectSingleNode(const DOMNode* parent, const std::string& localName)
{
    std::auto_ptr<DOMNodeList> nodes(parent->getChildNodes());
    for (unsigned long i = 0; i < nodes->getLength(); ++i)
    {
        DOMNode* node = nodes->getItem(i);
        if (node->getNamespaceURI() == namespaceURI
                && node->getLocalName() == localName)
            return node;

        delete node;
    }

    return NULL;
}

static double
parseDouble(const DOMNode* parent, const std::string& localName)
{
    std::auto_ptr<DOMNode> node(selectSingleNode(parent, localName));
    if (! node.get())
        return 0.;
    return SPI::atof(node->getNodeValue());
}

static void
parseLatLon(const DOMNode* parent, double& latitude, double& longitude)
{
    latitude = parseDouble(parent, "latitude");
    longitude = parseDouble(parent, "longitude");
}

/**********************************************************************/
/* parseErrorRS                                                       */
/**********************************************************************/

/*static*/ bool
Protocol::parseErrorRS(const DOMDocument* doc,
                       std::string& error)
{
    std::auto_ptr<DOMNode> docElement(doc->getDocumentElement());
    if (docElement.get() == NULL)
        return false;

    std::auto_ptr<DOMNode> errorElement(selectSingleNode(docElement.get(),
                                                         "error"));
    if (errorElement.get() == NULL)
        return false;

    error = errorElement->getNodeValue();
    return true;
}

/**********************************************************************/
/* parseLocationRS                                                    */
/**********************************************************************/

/*static*/ bool
Protocol::parseLocationRS(const DOMDocument* doc,
                          unsigned long timeDelta,
                          std::vector<LiteLocation>& locations)
{
    if (doc == NULL)
        return false;

    std::auto_ptr<DOMNode> docElement(doc->getDocumentElement());
    if (docElement.get() == NULL)
    {
        // don't check for hasErrorNode()
        // it's redundant
        return false;
    }

    std::auto_ptr<DOMNodeList> nodes(docElement.get()->getChildNodes());
    for (unsigned long i = 0; i < nodes->getLength(); ++i)
    {
        std::auto_ptr<DOMNode> node(nodes->getItem(i));
        if (node->getNamespaceURI() == namespaceURI
                && node->getLocalName() == "location")
        {
            LiteLocation location;

            location.hpe = parseDouble(node.get(), "hpe");
            location.nap = SPI::atoi(node->getAttributeNS("", "nap"));
            location.nsat = SPI::atoi(node->getAttributeNS("", "nsat"));
            location.ncell = SPI::atoi(node->getAttributeNS("", "ncell"));
            location.nlac = SPI::atoi(node->getAttributeNS("", "nlac"));

            const unsigned long age = SPI::atoi(node->getAttributeNS("", "age"));

            const long rqtime = SPI::atoi(node->getAttributeNS("", "rqtime"));
            if (rqtime > 0)
            {
                // Note that if the time on the device where the token is
                // saved is not synchronized with the one where the token
                // is restored, then this time delta will not be
                // correct. We fix negative times to be zero, but we leave
                // very large times unchanged.
                const long now = Time::now().sec();
                timeDelta = std::max(now - rqtime, 0L) * 1000;
            }

            location.time.reset(age + timeDelta);

            parseLatLon(node.get(), location.latitude, location.longitude);

            locations.push_back(location);
        }
    }

    if (locations.size() == 0)
        return false;

    return true;
}

}
}
