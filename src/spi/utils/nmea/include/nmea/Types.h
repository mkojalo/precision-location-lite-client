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

#ifndef WPS_SPI_NMEA_TYPES_H_
#define WPS_SPI_NMEA_TYPES_H_

namespace WPS {
namespace SPI {
namespace NMEA {

enum FixType
{
    FIX_TYPE_BAD = 1,
    FIX_TYPE_2D  = 2,
    FIX_TYPE_3D  = 3
};

enum FixQuality
{
    FIX_QUALITY_BAD       = 0, // Fix not available
    FIX_QUALITY_SPS       = 1, // GPS SPS Fix (Standard Positioning Service)
    FIX_QUALITY_DGPS      = 2, // Differential GPS Fix
    FIX_QUALITY_PPS       = 3, // PPS Fix (Precise Positioning Service)
    FIX_QUALITY_RTK       = 4, // Real Time Kinematic
    FIX_QUALITY_FRTK      = 5, // Float RTK
    FIX_QUALITY_ESTIMATED = 6, // Estimated (dead reckoning)
    FIX_QUALITY_MANUAL    = 7, // Manual Input Mode
    FIX_QUALITY_SIM       = 8  // Simulation
};

struct Satellite
{
    int prn;
    int elevation;
    int azimuth;
    int snr;
};

struct Time
{
    int hour;
    int minute;
    int second;
    int hsecond; // hundredth's of second
};

struct Date
{
    int day;
    int month;
    int year;
};

}
}
}

#endif
