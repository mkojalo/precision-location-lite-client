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

#include "Wrappers.h"

#include <cstring>

#include "spi/StdLibC.h"
#include "spi/Assert.h"

namespace WPS {
namespace API {

static const double MS_TO_KMH = 3.6;
static const double KMH_TO_MS = 1.0 / MS_TO_KMH;

inline double
convertSpeed(double speed, double f)
{
    if (speed == -1)
        return -1;

    assert(speed >= 0);
    return speed * f;
}

LiteLocation::operator SHLC_Location*() const
{
    SHLC_Location* location = new SHLC_Location;

    location->latitude = latitude;
    location->longitude = longitude;
    location->altitude = altitude;
    location->type = type;
    location->hpe = hpe;
    location->nap = nap;
    location->nsat = nsat;
    location->ncell = ncell;
    location->nlac = nlac;
    location->speed = convertSpeed(speed, MS_TO_KMH);
    location->bearing = bearing;
    location->age = time.elapsed();

    return location;
}

void
LiteLocation::free_location(SHLC_Location* p)
{
    delete p;
}

std::string
LiteLocation::toString(const SPI::Timer* timer) const
{
    char buf[128];
    int n = 0;

    n += WPS::SPI::snprintf(buf + n,
                            sizeof(buf) - n,
                            "%f, %f",
                            latitude,
                            longitude);

    if (type == SHLC_LOCATION_TYPE_3D)
    {
        n += WPS::SPI::snprintf(buf + n,
                                sizeof(buf) - n,
                                ", %f (3D)",
                                altitude);
    }
    else
    {
        n += WPS::SPI::snprintf(buf + n,
                                sizeof(buf) - n,
                                " (2D)");
    }

    n += WPS::SPI::snprintf(buf + n,
                            sizeof(buf) - n,
                            " +/-%.1fm (%d+%d+%d+%d)",
                            hpe,
                            nap,
                            nsat,
                            ncell,
                            nlac);

    if (speed != -1)
        n += SPI::snprintf(buf + n, sizeof(buf) - n, " %.2fm/s", speed);

    if (bearing != -1)
        n += SPI::snprintf(buf + n, sizeof(buf) - n, " %.2f", bearing);

    n += SPI::snprintf(buf + n, sizeof(buf) - n, " %lums", time.elapsed());

    if (timer)
        n += SPI::snprintf(buf + n, sizeof(buf) - n, " (elapsed %lums)", timer->elapsed());

    return buf;
}

}
}
