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

#ifndef WPS_API_WRAPPERS_H_
#define WPS_API_WRAPPERS_H_

#include "api/skyhookliteclient.h"

#include "spi/Time.h"
#include "spi/StdLibC.h"
#include "spi/ScannedAccessPoint.h"
#include "spi/ScannedCellTower.h"
#include "spi/GPSData.h"

#include <string>
#include <vector>

#include "spi/Assert.h"

namespace WPS {
namespace API {

struct Scan
{
    std::vector<SPI::ScannedAccessPoint> aps;
    std::vector<SPI::ScannedCellTower> cells;
    std::vector<SPI::GPSData::Fix> gps;
};

/**
 * C++ version of \c SHLC_Location
 *
 * @see SHLC_Location
 */
struct LiteLocation
{
    double latitude;
    double longitude;
    double altitude;
    SHLC_LocationType type;
    double hpe;
    unsigned short nap;
    unsigned short nsat;
    unsigned short ncell;
    unsigned short nlac;
    double speed;
    double bearing;
    SPI::Timer time;

    LiteLocation()
        : latitude(0)
        , longitude(0)
        , altitude(0)
        , type(SHLC_LOCATION_TYPE_2D)
        , hpe(0)
        , nap(0)
        , nsat(0)
        , ncell(0)
        , nlac(0)
        , speed(-1)
        , bearing(-1)
    {}

    operator SHLC_Location*() const;

    static void free_location(SHLC_Location*);

    std::string toString(const SPI::Timer* timer = NULL) const;
};

}
}

#endif
