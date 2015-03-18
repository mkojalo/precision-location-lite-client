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

#ifndef WPS_SPI_NMEA_INFO_H_
#define WPS_SPI_NMEA_INFO_H_

#include "Types.h"
#include "Dataset.h"
#include "Units.h"

#include <vector>

#include "spi/Assert.h"

namespace WPS {
namespace SPI {
namespace NMEA {

typedef std::vector<Satellite> SatsInView;
typedef unsigned char SatsInUse[MAX_SAT_IN_USE];

class Info
    : public Dataset
{
public:

    Info()
    {
        set(FIX_MODE, 'A');
        setFixType(FIX_TYPE_BAD);
        setFixQuality(FIX_QUALITY_BAD);
    }

    /*******************************************/
    /* Setters                                 */
    /*******************************************/

    void setFixType(FixType fixType)
    {
        set(FIX_TYPE, fixType);
    }

    void setFixQuality(FixQuality fixQuality)
    {
        set(FIX_QUALITY, fixQuality);
        set(FIX_MODE_INDICATOR, fixQuality > FIX_QUALITY_BAD ? 'A' : 'N');
        set(STATUS, fixQuality > FIX_QUALITY_BAD ? 'A' : 'V');
    }

    void setLatitude(double latitude)
    {
        set(LATITUDE, Math::fabs(latitude));
        set(LATITUDE_NS, latitude > 0 ? 'N' : 'S');
    }

    void setLongitude(double longitude)
    {
        set(LONGITUDE, Math::fabs(longitude));
        set(LONGITUDE_EW, longitude > 0 ? 'E' : 'W');
    }

    void setAltitude(double altitude)
    {
        set(ALTITUDE, altitude);
        set(ALTITUDE_UNITS, 'M');
    }

    void setSpeed(double speedInKph)
    {
        set(SPEED, kph2knots(speedInKph));
    }

    void setDirection(double direction)
    {
        set(DIRECTION, direction);
    }

    void setDeclination(double declination)
    {
        set(DECLINATION, Math::fabs(declination));
        set(DECLINATION_EW, declination > 0 ? 'E' : 'W');
    }

    void setDiff(double diff)
    {
        set(DIFF, diff);
        set(DIFF_UNITS, 'M');
    }

    void setPDOP(double pdop)
    {
        set(PDOP, pdop);
    }

    void setHDOP(double hdop)
    {
        set(HDOP, hdop);
    }

    void setVDOP(double vdop)
    {
        set(VDOP, vdop);
    }

    void setTime(const Time& time)
    {
        set(TIME, time);
    }

    void setDate(const Date& date)
    {
        set(DATE, date);
    }

    void setSatsInView(const SatsInView& satsInView)
    {
        assert(satsInView.size() <= MAX_SAT_IN_VIEW);

        for (int i = 0; i < static_cast<int>(satsInView.size()); ++i)
        {
            const Satellite& sat = satsInView[i];

            // See http://www.us-technology.co.kr/product/doc/gps.pdf
            assert(sat.elevation >= -90 && sat.elevation <= 90);
            assert(sat.azimuth >= 0 && sat.azimuth <= 359);
            assert(sat.snr >= 0 && sat.snr <= 99);

            // In addition to the 1-32 satellite range we also handle pseudo-satellites (33-51).
            // See http://gpsinformation.net/exe/waas.html
            assert(sat.prn >= 1 && sat.prn <= 51);

            set(SATELLITE_01 + i, sat);
        }

        set(SAT_IN_VIEW, static_cast<int>(satsInView.size()));
    }

    void setSatsInUse(const SatsInUse& satsInUse)
    {
        int n = 0;

        for (int i = 0; i < MAX_SAT_IN_USE; ++i)
        {
            if (satsInUse[i] != 0)
            {
                // Pseudolites are reported with ID 255 on the Datalogic prototype
                assert(satsInUse[i] >= 1 && satsInUse[i] <= 255);

                set(SAT_IN_USE_01 + i, satsInUse[i]);
                ++n;
            }
        }

        set(SAT_IN_USE, n);
    }

    void clearSatsInView()
    {
        set(SAT_IN_VIEW, 0);
        for (int i = 0; i < MAX_SAT_IN_VIEW; ++i)
            remove(SATELLITE_01 + i);
    }

    void clearSatsInUse()
    {
        set(SAT_IN_USE, 0);
        for (int i = 0; i < MAX_SAT_IN_USE; ++i)
            remove(SAT_IN_USE_01 + i);
    }

    void clearSatellites()
    {
        clearSatsInView();
        clearSatsInUse();
    }

    void setDGPSTime(double DGPSTime)
    {
        set(DGPS_TIME, DGPSTime);
    }

    void setDGPSId(int DGPSId)
    {
        set(DGPS_ID, DGPSId);
    }

    /*******************************************/
    /* Getters                                 */
    /*******************************************/

    FixType getFixType() const
    {
        const int fixType = get(FIX_TYPE);
        if (!fixType)
            return FIX_TYPE_BAD;

        return static_cast<FixType>(fixType);
    }

    FixQuality getFixQuality() const
    {
        const int fixQuality = get(FIX_QUALITY);
        return static_cast<FixQuality>(fixQuality);
    }

    double getLatitude() const
    {
        const char ns = get(LATITUDE_NS);
        const double latitude = get(LATITUDE);
        return ns == 'N' ? latitude : -latitude;
    }

    double getLongitude() const
    {
        const char ew = get(LONGITUDE_EW);
        const double lontitude = get(LONGITUDE);
        return ew == 'E' ? lontitude : -lontitude;
    }

    double getAltitude() const
    {
        return get(ALTITUDE);
    }

    double getSpeed() const
    {
        return knots2kph(get(SPEED));
    }

    double getDirection() const
    {
        return get(DIRECTION);
    }

    double getDeclination() const
    {
        const char ew = get(DECLINATION_EW);
        const double declination = get(DECLINATION);
        return ew == 'E' ? declination : -declination;
    }

    double getDiff() const
    {
        return get(DIFF);
    }

    double getPDOP() const
    {
        return get(PDOP);
    }

    double getHDOP() const
    {
        return get(HDOP);
    }

    double getVDOP() const
    {
        return get(VDOP);
    }

    Time getTime() const
    {
        return get(TIME);
    }

    Date getDate() const
    {
        return get(DATE);
    }

    int getSatsInView() const
    {
        // Don't trust SAT_IN_VIEW in GSV but calculate
        // the actual number of satellites.
        // Some devices (Gobi 2000) may emit incomplete satellite records
        // in GSV (lacking azimuth, elevation or SNR) and those
        // would be filtered out when parsing.

        int n = 0;
        for (int i = 0; i < MAX_SAT_IN_VIEW; ++i)
            if (isPresent(SATELLITE_01 + i))
                ++n;

        return n;
    }

    int getSatsInUse() const
    {
        // We don't trust SAT_IN_USE from GGA and calculate it based on GSA.
        // Some devices (Broadcom) may emit wrong number in GGA.
        // In addition GSA could be the only sentence parsed and this method should still work.

        int n = 0;
        for (int i = 0; i < MAX_SAT_IN_USE; ++i)
            if (isPresent(SAT_IN_USE_01 + i))
                ++n;

        return n;
    }

    void getSatsInView(SatsInView& satsInView) const
    {
        for (int i = 0; i < MAX_SAT_IN_VIEW; ++i)
        {
            if (isPresent(SATELLITE_01 + i))
                satsInView.push_back(get(SATELLITE_01 + i));
        }
    }

    void getSatsInUse(SatsInUse& satsInUse)
    {
        for (int i = 0; i < MAX_SAT_IN_USE; ++i)
        {
            if (isPresent(SAT_IN_USE_01 + i))
                satsInUse[i] = static_cast<int>(get(SAT_IN_USE_01 + i));
            else
                satsInUse[i] = 0;
        }
    }

    double getDGPSTime() const
    {
        return get(DGPS_TIME);
    }

    int getDGPSId() const
    {
        return get(DGPS_ID);
    }

    /*******************************************/
    /* Copiers                                 */
    /*******************************************/

    void copyFixInformation(Info& to) const
    {
        copy(FIX_TYPE, to);
        copy(FIX_QUALITY, to);
        copy(FIX_MODE_INDICATOR, to);
        copy(STATUS, to);
    }

    void copyLatitude(Info& to) const
    {
        copy(LATITUDE, to);
        copy(LATITUDE_NS, to);
    }

    void copyLongitude(Info& to) const
    {
        copy(LONGITUDE, to);
        copy(LONGITUDE_EW, to);
    }

    void copyAltitude(Info& to) const
    {
        copy(ALTITUDE, to);
        copy(ALTITUDE_UNITS, to);
    }

    void copySpeed(Info& to) const
    {
        copy(SPEED, to);
    }

    void copyDirection(Info& to) const
    {
        copy(DIRECTION, to);
    }

    void copyDeclination(Info& to) const
    {
        copy(DECLINATION, to);
        copy(DECLINATION_EW, to);
    }

    void copyDiff(Info& to) const
    {
        copy(DIFF, to);
        copy(DIFF_UNITS, to);
    }

    void copyDGPS(Info& to) const
    {
        copy(DGPS_TIME, to);
        copy(DGPS_ID, to);
    }

    void copyDateTime(Info& to) const
    {
        copy(DATE, to);
        copy(TIME, to);
    }

    void copySatellites(Info& to) const
    {
        copy(SAT_IN_USE, to);
        copy(SAT_IN_VIEW, to);

        for (int i = 0; i < MAX_SAT_IN_VIEW; ++i)
            copy(SATELLITE_01 + i, to);

        for (int i = 0; i < MAX_SAT_IN_USE; ++i)
            copy(SAT_IN_USE_01 + i, to);
    }
};

}
}
}

#endif
