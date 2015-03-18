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

#ifndef WPS_SPI_NMEA_FIELDS_H_
#define WPS_SPI_NMEA_FIELDS_H_

namespace WPS {
namespace SPI {
namespace NMEA {

const int MAX_SAT_IN_USE = 12;
const int MAX_SAT_IN_VIEW = 16;

enum
{
    FIX_TYPE,
    FIX_QUALITY,
    FIX_MODE,           // [M]anual, forced to operate in 2D or 3D; [A]utomatic, 3D/2D

    LATITUDE,
    LATITUDE_NS,        // [N]orth or [S]outh
    LONGITUDE,
    LONGITUDE_EW,       // [E]ast or [W]est
    ALTITUDE,           // Antenna altitude above/below mean sea level (geoid)
    ALTITUDE_UNITS,     // [M]eters (Antenna height unit)
    SPEED,              // Speed over the ground in knots
    DIRECTION,          // Track angle in degrees
    DECLINATION,        // Magnetic variation degrees
    DECLINATION_EW,     // [E]ast or [W]est
    DIFF,               // Geoidal separation
    DIFF_UNITS,         // [M]eters (Units of geoidal separation)
    FIX_MODE_INDICATOR, // Mode indicator of fix type:
                        //     'A' - autonomous
                        //     'D' - differential
                        //     'E' - estimated
                        //     'N' - not valid
                        //     'S' - simulator

    STATUS,             // A - OK, V - Navigation receiver warning

    PDOP,               // Dilution of precision
    HDOP,               // Horizontal dilution of precision
    VDOP,               // Vertical dilution of precision

    TIME,
    DATE,

    DGPS_TIME,          // Time in seconds since last DGPS update
    DGPS_ID,            // DGPS station ID number

    SAT_IN_VIEW,        // Number of satellites in view
    SAT_IN_USE,         // Number of satellites in use

    SAT_IN_USE_01,
    SAT_IN_USE_02,
    SAT_IN_USE_03,
    SAT_IN_USE_04,
    SAT_IN_USE_05,
    SAT_IN_USE_06,
    SAT_IN_USE_07,
    SAT_IN_USE_08,
    SAT_IN_USE_09,
    SAT_IN_USE_10,
    SAT_IN_USE_11,
    SAT_IN_USE_12,

    SATELLITE_01,
    SATELLITE_02,
    SATELLITE_03,
    SATELLITE_04,
    SATELLITE_05,
    SATELLITE_06,
    SATELLITE_07,
    SATELLITE_08,
    SATELLITE_09,
    SATELLITE_10,
    SATELLITE_11,
    SATELLITE_12,
    SATELLITE_13,       // Satellites 13-16 are available on 16-channel GPS units only
    SATELLITE_14,
    SATELLITE_15,
    SATELLITE_16
};

}
}
}

#endif
