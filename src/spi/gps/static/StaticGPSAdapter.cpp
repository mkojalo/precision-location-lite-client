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

#include "spi/GPSAdapter.h"
#include "spi/Time.h"

namespace WPS {
namespace SPI {
    
/*********************************************************************/
/*                                                                   */
/* StaticGPSAdapter                                                  */
/*                                                                   */
/* NOTE: this adapter returns static data and is for testing         */
/*       purposes only                                               */
/*                                                                   */
/*********************************************************************/

class StaticGPSAdapter
    : public GPSAdapter
{
public:

    StaticGPSAdapter()
    {}

    std::string description() const
    {
        return "StaticGPSAdapter";
    }
    
    void setListener(Listener* listener)
    {
        _listener = listener;
    }

    ErrorCode open()
    {
        const unsigned char SATS = 5;

        const Time now = Time::now();
        
        GPSData gpsData;

        GPSData::Fix* fix = new GPSData::Fix;

        fix->quality = 1;
        fix->latitude = 42.349983;
        fix->longitude = -71.047798;
        fix->height = 90.0;
        fix->altitude = 100.0;

        fix->hdop = 3.0f;
        fix->hpe = 10;
        fix->gpsTime = now;
        fix->timetag = now.sec();
        fix->svInFix = SATS;

        short azimuth = 0;

        for (unsigned char i = 0; i < SATS; ++i)
        {
            fix->prn[i] = 31 + i;

            azimuth = (azimuth + 360 / SATS) % 360;

            GPSData::Satellite s;
            s.satelliteId = 31 + i;
            s.timetag = now.sec();
            s.azimuth = azimuth;
            s.elevation = 50;
            s.snr = 27;

            gpsData.satellites.push_back(s);
        }

        gpsData.fix.reset(fix);

        _listener->onGpsData(gpsData);

        return SPI_OK;
    }

    void close()
    {}

private:

    Listener* _listener;
};

/*********************************************************************/
/*                                                                   */
/* GPSAdapter::newInstance                                           */
/*                                                                   */
/*********************************************************************/

GPSAdapter*
GPSAdapter::newInstance()
{
    return new StaticGPSAdapter;
}

}
}
