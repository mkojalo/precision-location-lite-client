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

#ifndef WPS_SPI_SCANNED_ACCESS_POINT_H_
#define WPS_SPI_SCANNED_ACCESS_POINT_H_

#include <algorithm>
#include <functional>
#include <string>
#include <vector>

#include "spi/MAC.h"
#include "spi/Time.h"
#include "spi/Assert.h"

namespace WPS {
namespace SPI {

/**
 * \ingroup nonreplaceable
 *
 * Encapsulate a scanned access point, made out of a MAC address,
 * a power reading (RSSI - Received Signal Strength Indication)
 * and a time-stamp.
 *
 * @see http://en.wikipedia.org/wiki/Rssi
 *
 * @author Skyhook Wireless
 */
class ScannedAccessPoint
{
public:

    typedef std::vector<unsigned char> SSID;

    /**
     * Creates a new instance with a <code>mac</code> address,
     * a power reading (<code>rssi</code>), and a time-stamp.
     *
     * @param mac the MAC address of the access point.
     * @param rssi the received signal strength in decibel
     *             or in percentage of max.
     * @param timestamp the time when the reading was captured.
     * @param ssid the SSID of the access point.
     */
    ScannedAccessPoint(const MAC& mac,
                       short rssi,
                       const Timer& timestamp,
                       const SSID& ssid = SSID())
        : _mac(mac)
        , _rssi(todBm(rssi))
        , _timestamp(timestamp)
        , _ssid(ssid)
    {
        assert(-255 <= _rssi && _rssi <= 0);
    }

    ScannedAccessPoint(const ScannedAccessPoint& that)
        : _mac(that._mac)
        , _rssi(that._rssi)
        , _timestamp(that._timestamp)
        , _ssid(that._ssid)
    {}

    ScannedAccessPoint& operator=(const ScannedAccessPoint& that)
    {
        if (this != &that)
        {
            _mac = that._mac;
            _rssi = that._rssi;
            _timestamp = that._timestamp;
            _ssid = that._ssid;
        }
        return *this;
    }

    ~ScannedAccessPoint()
    {}

    int compare(const ScannedAccessPoint& that) const
    {
        // NOTE: we don't compare _rssi nor _ssid here because
        //       _mac and _timestamp is enough to uniquely identify
        //       an AP scanned on a device with one Wifi adapter.

        if (const int r = _mac.compare(that._mac))
            return r;

        // Make newer readings sort first
        return _timestamp.compare(that._timestamp);
    }

    const MAC& getMAC() const
    {
        return _mac;
    }

    short getRSSI() const
    {
        return _rssi;
    }

    const Timer& getTimestamp() const
    {
        return _timestamp;
    }

    const SSID& getSsid() const
    {
        return _ssid;
    }

    /**
     * @return <code>this</code> scanned access point as a string.
     */
    std::string toString() const
    {
        return _mac.toString()
             + "," + toAsciiString(_ssid)
             + "," + itoa(_rssi)
             + "," + _timestamp.toString();
    }

    static char toAsciiChar(unsigned char c)
    {
        return c >= 0x20 && c <= 0x7F ? c : '?';
    }

    static std::string toAsciiString(const std::vector<unsigned char>& v)
    {
        std::string s(v.size(), 0);
        std::transform(v.begin(), v.end(), s.begin(), toAsciiChar);
        return s;
    }

    struct MacSame
        : std::binary_function<ScannedAccessPoint, ScannedAccessPoint, bool>
    {
        bool operator()(const ScannedAccessPoint& lhs,
                        const ScannedAccessPoint& rhs) const
        {
            return lhs.getMAC().compare(rhs.getMAC()) == 0;
        }
    };

    struct MacLess
        : std::binary_function<ScannedAccessPoint, ScannedAccessPoint, bool>
    {
        bool operator()(const ScannedAccessPoint& lhs,
                        const ScannedAccessPoint& rhs) const
        {
            return lhs.getMAC().compare(rhs.getMAC()) < 0;
        }
    };

    struct MacEqualsTo
        : std::binary_function<ScannedAccessPoint, MAC, bool>
    {
        bool operator()(const ScannedAccessPoint& ap,
                        const MAC& mac) const
        {
            return ap.getMAC() == mac;
        }
    };

    struct WeakerRssi
        : std::binary_function<ScannedAccessPoint, ScannedAccessPoint, bool>
    {
        bool operator()(const ScannedAccessPoint& lhs,
                        const ScannedAccessPoint& rhs) const
        {
            return lhs.getRSSI() < rhs.getRSSI();
        }
    };

private:

    short todBm(short rssi)
    {
        if (rssi < 0 && rssi > -255)
        {
            // assume rssi is in dBm
            return rssi;
        }
        else if (rssi > 0 && rssi < 100)
        {
            // assume rssi is in percentage
            // convert to dBm
            return rssi * 69 / 100 - 90;
        }
        else
        {
            // invalid rssi
            return -255;
        }
    }

private:

    MAC _mac;
    short _rssi;
    Timer _timestamp;

    SSID _ssid;
};

}
}

#endif
