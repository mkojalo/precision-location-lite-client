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

#ifndef WPS_SPI_GPS_DATA_H_
#define WPS_SPI_GPS_DATA_H_

#include "spi/Time.h"
#include "spi/StdLibC.h"

#include <limits>
#include <memory>
#include <vector>

namespace WPS {
namespace SPI {

const unsigned MAX_SAT_IN_USE = 12;

/**
 * \ingroup nonreplaceable
 *
 * Encapsulate GPS data.
 *
 * @since WPS API 3.0
 *
 * @author Skyhook Wireless
 */
struct GPSData
{
    /**
     * A GPS fix
     */
    struct Fix
    {
        Fix()
            : quality(0)
            , latitude(0)
            , longitude(0)
            , height(std::numeric_limits<double>::max())
            , altitude(std::numeric_limits<double>::max())
            , speed(std::numeric_limits<double>::max())
            , bearing(std::numeric_limits<double>::max())
            , hdop(0)
            , hpe(0)
            , timetag(0)
            , svInFix(0)
        {
            for (unsigned i = 0; i < sizeof(prn); ++i)
                prn[i] = 0;
        }

        int compare(const Fix& that) const;

        bool hasAltitude() const
        {
            return altitude != std::numeric_limits<double>::max();
        }

        bool hasHeight() const
        {
            return height != std::numeric_limits<double>::max();
        }

        bool hasSpeed() const
        {
            return speed != std::numeric_limits<double>::max();
        }

        bool hasBearing() const
        {
            return bearing != std::numeric_limits<double>::max();
        }

        bool hasHdop() const
        {
            return hdop != 0;
        }

        bool hasHpe() const
        {
            return hpe != 0;
        }

        /**
         * Fix quality:
         *   0 - Invalid
         *   1 - GPS SPS Fix (Standard Positioning Service)
         *   2 - Differential GPS Fix
         *   3 - PPS Fix (Precise Positioning Service)
         *   4 - Real Time Kinematic
         *   5 - Float RTK
         *   6 - Estimated (dead reckoning)
         *   7 - Manual Input Mode
         *   8 - Simulation
         */
        unsigned char quality;

        // @{
        /**
         * Coordinates (in decimal degrees)
         */
        double latitude;
        double longitude;
        // @}

        /**
         * Altitude above WGS84 ellipsoid (in meters)
         */
        double height;

        /**
         * Altitude above MSL (in meters)
         */
        double altitude;

        /**
         * Absolute velocity (expressed in meters/second)
         */
        double speed;

        /**
         * Bearing (expressed in degrees from north).
         * Set to zero if not available.
         */
        double bearing;

        /**
         * Horizontal dilution of precision.
         * Set to zero if the fix is not satellite based.
         */
        float hdop;

        /**
         * Horizontal position error (in meters)
         */
        float hpe;

        /**
         * Local time of fix
         */
        Timer localTime;

        /**
         * GPS time of fix
         */
        Time gpsTime;

        /**
         * Time tag when the fix was acquired (in seconds)
         */
        unsigned long timetag;

        /**
         * Satellites used in the fix.
         * Set to zero if the fix is extrapolated or not satellite based.
         */
        unsigned char svInFix;

        /**
         * PRN of satellites used in the fix
         */
        unsigned char prn[MAX_SAT_IN_USE];

        /**
         * @return <code>this</code> GPS fix as a string.
         */
        std::string toString() const;
    };

    /**
     * Information about a GPS satellite
     */
    struct Satellite
    {
        Satellite()
            : satelliteId(0),
              azimuth(0),
              elevation(0),
              snr(0)
        {}

        /**
         * Satellite PRN
         */
        unsigned char satelliteId;

        /**
         * Time tag when the measurement was taken (in seconds)
         */
        double timetag;

        // @{
        /**
         * Satellite angle coordinates
         */
        unsigned short azimuth;
        short elevation;
        // @}

        /**
         * Signal to Noise Ratio (in dB)
         */
        unsigned char snr;

        /**
         * @return <code>this</code> GPS satellite as a string.
         */
        std::string toString() const
        {
            char buf[32];
            snprintf(buf,
                     sizeof buf,
                     "%hhu:%hhd,%d,%u",
                     satelliteId,
                     snr,
                     elevation,
                     azimuth);
            return buf;
        }
    };

    GPSData(const Fix& fix,
            const std::vector<Satellite>& satellites)
        : fix(new Fix(fix))
        , satellites(satellites)
    {}

    GPSData(const std::vector<Satellite>& satellites)
        : satellites(satellites)
    {}

    GPSData(const Fix& fix)
        : fix(new Fix(fix))
    {}

    GPSData()
    {}

    GPSData(const GPSData& other)
        : fix(other.fix.get() ? new Fix(*other.fix) : NULL)
        , satellites(other.satellites)
    {}

    GPSData& operator=(const GPSData& other)
    {
        if (&other != this)
        {
            fix.reset(other.fix.get() ? new Fix(*other.fix) : NULL);
            satellites = other.satellites;
        }
        return *this;
    }

    bool empty() const
    {
        return fix.get() == NULL && satellites.empty();
    }

    void clear()
    {
        fix.reset();
        satellites.clear();
    }

    std::auto_ptr<Fix> fix;
    std::vector<Satellite> satellites;
};

}
}

#endif
