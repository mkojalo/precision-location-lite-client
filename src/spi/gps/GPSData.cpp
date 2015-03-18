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

#include "spi/GPSData.h"

namespace WPS {
namespace SPI {

template <typename T>
static int simpleCompare(const T& lhs, const T& rhs)
{
    if (lhs < rhs)
        return -1;

    if (rhs < lhs)
        return 1;

    return 0;
}

int GPSData::Fix::compare(const Fix& that) const
{
    // Note that this doesn't compare prn

    // Note that we make newer readings sort first even though their
    // timestamp is larger by negating the return value

    if (const int r = localTime.compare(that.localTime))
        return - r;  // Note opposite sign

    if (const int r = gpsTime.compare(that.gpsTime))
        return - r;  // Note opposite sign

    if (const int r = simpleCompare(timetag, that.timetag))
        return - r;  // Note opposite sign

    if (const int r = simpleCompare(latitude, that.latitude))
        return r;

    if (const int r = simpleCompare(longitude, that.longitude))
        return r;

    if (const int r = svInFix - that.svInFix)
        return r;

    if (const int r = static_cast<int>(hpe) - static_cast<int>(that.hpe))
        return r;

    if (const int r = simpleCompare(hdop, that.hdop))
        return r;

    // Note that for optional fields if the field is not set, then it
    // is considered less than if it the field is set. If the field is
    // not set in either one, then it is safe to compare the value
    // since it will always be the same if it is not set to a real
    // value.

    if (hasAltitude() ^ that.hasAltitude())
        return hasAltitude() ? 1 : -1;

    if (const int r = simpleCompare(altitude, that.altitude))
        return r;

    if (hasHeight() ^ that.hasHeight())
        return hasHeight() ? 1 : -1;

    if (const int r = simpleCompare(height, that.height))
        return r;

    if (hasSpeed() ^ that.hasSpeed())
        return hasSpeed() ? 1 : -1;

    if (const int r = simpleCompare(speed, that.speed))
        return r;

    if (hasBearing() ^ that.hasBearing())
        return hasBearing() ? 1 : -1;

    if (const int r = simpleCompare(bearing, that.bearing))
        return r;

    return quality - that.quality;
}

std::string GPSData::Fix::toString() const
{
    std::string s;
    char buf[128];

    snprintf(buf, sizeof(buf), "%f, %f", latitude, longitude);
    s.append(buf);

    if (hasAltitude())
    {
        snprintf(buf, sizeof(buf), ", %f", altitude);
        s.append(buf);
        s.append(" (3D)");
    }
    else
        s.append(" (2D)");

    snprintf(buf, sizeof(buf), " %.2f", hdop);
    s.append(buf);

    if (hpe != 0)
    {
        snprintf(buf, sizeof(buf), " +/-%.1fm", hpe);
        s.append(buf);
    }

    snprintf(buf, sizeof(buf), " (%d)", svInFix);
    s.append(buf);

    if (hasSpeed())
    {
        snprintf(buf, sizeof(buf), " %.2fm/s", speed);
        s.append(buf);
    }

    if (hasBearing())
    {
        snprintf(buf, sizeof(buf), " %.2f", bearing);
        s.append(buf);
    }

    return s;
}

}
}
