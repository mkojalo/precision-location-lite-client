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

#include "SirfProtocol.h"

#include "spi/StdLibC.h"
#include "spi/Time.h"

#include <vector>
#include <stdint.h>

#include "spi/Assert.h"

namespace WPS {
namespace SPI {

static const uint16_t SIRF_HEADER = 0xA0A2;
static const uint16_t SIRF_FOOTER = 0xB0B3;

static const size_t SIRF_AUX_SIZE = 8; // header + payload length + checksum + footer
static const size_t SIRF_MIN_SIZE = SIRF_AUX_SIZE + 2; // aux + message id

static const time_t GPS_TIME_OFFSET = 315964800; // 6 January 1980

static uint16_t
sirfChecksum(const uint8_t* payload, size_t size)
{
    assert(size > 0);
    assert(payload);

    uint16_t checksum = 0;
    for (size_t i = 0; i < size; ++i)
        checksum += payload[i];

    return checksum & 0x7FFF;
}

/***********************************************************************/
/* Input messages that we are receiving from the GPS device            */
/*                                                                     */
/* Note: for the device side they are output messages                  */
/* See "Ch3 - Output Messages" in the reference manual                 */
/***********************************************************************/

class SirfInputStream
{
public:

    SirfInputStream(const uint8_t* p, size_t size)
        : _origin(p), _p(p), _size(size)
    {
        assert(size > 0);
    }

    uint8_t read8()
    {
        assertAvail(sizeof(uint8_t));
        uint8_t n = *_p++;
        return n;
    }

    uint16_t read16()
    {
        assertAvail(2);

        uint16_t n;
        char* pn = (char*) &n;

        pn[0] = _p[1];
        pn[1] = _p[0];

        _p += 2;

        return n;
    }

    int16_t read16s()
    {
        assertAvail(2);

        int16_t n;
        char* pn = (char*) &n;

        pn[0] = _p[1];
        pn[1] = _p[0];

        _p += 2;

        return n;
    }

    uint32_t read32()
    {
        assertAvail(4);

        uint32_t n;
        char* pn = (char*) &n;

        pn[0] = _p[3];
        pn[1] = _p[2];
        pn[2] = _p[1];
        pn[3] = _p[0];

        _p += 4;

        return n;
    }

    int32_t read32s()
    {
        assertAvail(4);

        int32_t n;
        char* pn = (char*) &n;

        pn[0] = _p[3];
        pn[1] = _p[2];
        pn[2] = _p[1];
        pn[3] = _p[0];

        _p += 4;

        return n;
    }

    float readFloat()
    {
        assertAvail(sizeof(float));

        float n;
        char* pn = (char*) &n;

        pn[0] = _p[3];
        pn[1] = _p[2];
        pn[2] = _p[1];
        pn[3] = _p[0];

        _p += 4;

        return n;
    }

    double readDouble()
    {
        assertAvail(sizeof(double));

        double n;
        char* pn = (char*) &n;

        // see reference manual Message ID 28
        // for byte ordering description
        pn[0] = _p[3];
        pn[1] = _p[2];
        pn[2] = _p[1];
        pn[3] = _p[0];
        pn[4] = _p[7];
        pn[5] = _p[6];
        pn[6] = _p[5];
        pn[7] = _p[4];

        _p += 8;

        return n;
    }

    const uint8_t* skip(size_t n)
    {
        assertAvail(n);

        const uint8_t* p = _p;
        _p += n;

        return p;
    }

    void rewind(size_t n)
    {
        assert(offset() >= n);
        _p -= n;
    }

    uint16_t checksum() const
    {
        return sirfChecksum(_origin, _size);
    }

    size_t size() const
    {
        return _size;
    }

    size_t offset() const
    {
        return static_cast<size_t>(_p - _origin);
    }

    size_t avail() const
    {
        assert(offset() <= size());
        return size() - offset();
    }

private:

    void assertAvail(size_t size) const
    {
        assert(avail() >= size);
    }

private:

    const uint8_t* _origin;
    const uint8_t* _p;
    size_t _size;
};

/***********************************************************************/
/* Convert raw GPS time to SPI::Time                                   */
/***********************************************************************/

static Time
fromGPSTime(const uint16_t week, const uint32_t timeOfWeek)
{
    return Time(GPS_TIME_OFFSET
                    + week * 7 * 24 * 3600
                    + timeOfWeek,
                0);
}

/***********************************************************************/
/* Geodetic Navigation Data - Message ID 41                            */
/***********************************************************************/

static GPSData::Fix*
parseFix(SirfInputStream& payload)
{
    if (payload.size() != 91)
        return NULL;

    payload.skip(2);

    const uint16_t type = payload.read16();
    if ((type & 0x07) == 0)
        return NULL;

    GPSData::Fix* fix = new GPSData::Fix;

    fix->quality = 1;

    const uint16_t week = payload.read16();
    const uint32_t timeOfWeek = payload.read32() / 1000;

    fix->gpsTime = fromGPSTime(week, timeOfWeek);
    fix->timetag = fix->gpsTime.sec();

    payload.read16(); // skip year
    payload.read8();  // skip month
    payload.read8();  // skip day
    payload.read8();  // skip hour
    payload.read8();  // skip minute
    payload.read16(); // skip second

    const uint32_t satsMap = payload.read32();
    int satsInUse = 0;
    for (int i = 0; i < 32; ++i)
    {
        if ((satsMap >> i) & 1)
            fix->prn[satsInUse++] = i + 1;

        if (satsInUse == (sizeof(fix->prn) / sizeof(fix->prn[0])))
            break;
    }

    fix->svInFix = satsInUse;
    fix->latitude = payload.read32s();  
    fix->latitude /= 10000000;
    fix->longitude = payload.read32s();
    fix->longitude /= 10000000;
    fix->height = payload.read32s();
    fix->height /= 100;
    fix->altitude = payload.read32s();
    fix->altitude /= 100;

    payload.read8(); // skip map datum

    fix->speed = payload.read16();
    fix->speed /= 100;
    fix->bearing = payload.read16();
    fix->bearing /= 100;

    payload.read16(); // skip magnetic variation
    payload.read16(); // skip clim rate
    payload.read16(); // skip heading rate

    float hpe = static_cast<float>(payload.read32());
    hpe /= 100;
    if (hpe >= 8)  // don't trust HPE lower than 8m
        fix->hpe = hpe;

    payload.read32(); // skip vpe
    payload.read32(); // skip time error
    payload.read16(); // skip velocity error
    payload.read32(); // skip clock bias
    payload.read32(); // skip clock bias error
    payload.read32(); // skip clock drift
    payload.read32(); // skip clock drift error
    payload.read32(); // skip distance
    payload.read16(); // skip distance error
    payload.read16(); // skip bearing error
    payload.read8();  // skip sats in fix

    fix->hdop = payload.read8();
    fix->hdop /= 5;

    payload.read8(); // additional mode info

    assert(payload.avail() == 0);

    return fix;
}

/***********************************************************************/
/* Measured Tracker Data Out - Message ID 4                            */
/***********************************************************************/

static std::vector<GPSData::Satellite>
parseSatellites(SirfInputStream& payload)
{
    // No check for payload size so that we potentially
    // support more than 12 channels/satellites in the message

    std::vector<GPSData::Satellite> sats;

    const uint16_t week = payload.read16();
    const uint32_t timeOfWeek = payload.read32() / 100;
    const unsigned long timetag = fromGPSTime(week, timeOfWeek).sec();

    const uint8_t nsat = payload.read8();
    if (payload.avail() != nsat * 15) // 15 bytes per each channel/satellite
        return sats;

    for (unsigned i = 0; i < nsat; ++i)
    {
        GPSData::Satellite sat;
        sat.satelliteId = payload.read8();
        sat.azimuth = (unsigned short) ((double) payload.read8() * 3 / 2);
        sat.elevation = (short) ((double) payload.read8() / 2);

        payload.skip(2); // skip state
        payload.skip(9); // skip first 9 C/N0
        sat.snr = payload.read8();
        sat.timetag = timetag;

        if (sat.satelliteId)
            sats.push_back(sat);
    }

    assert(payload.avail() == 0);

    return sats;
}

size_t
SirfProtocol::tryParse(const char* data, size_t size)
{
    size_t parsedBytes = 0;

    SirfInputStream stream(reinterpret_cast<const uint8_t*>(data), size);

    while (stream.avail() >= SIRF_MIN_SIZE)
    {
        if (stream.read16() == SIRF_HEADER)
        {
            const uint16_t len = stream.read16();
            if (len + 4U > stream.avail() || len <= 0)
                continue;

            SirfInputStream payload(stream.skip(len), len);
            const uint16_t checksum = stream.read16();

            if (stream.read16() != SIRF_FOOTER)
                continue;

            if (checksum != payload.checksum())
                continue;

            parsedBytes = stream.offset();

            // Recognized and validated the SiRF message,
            // can now proceed with parsing the payload
            const uint8_t id = payload.read8();

            if (_logger.isDebugEnabled())
                _logger.debug("received SiRF message #%u", id);

            switch (id)
            {
            case 41: // Geodetic Navigation Data - Message ID 41
                _data.fix.reset(parseFix(payload));
                break;

            case 4: // Measured Tracker Data Out - Message ID 4
                _data.satellites = parseSatellites(payload);
                break;
            }
        }
        else
            stream.rewind(1);
    }

    return parsedBytes;
}

}
}
