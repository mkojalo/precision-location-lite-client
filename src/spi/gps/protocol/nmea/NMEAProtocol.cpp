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

#include "NMEAProtocol.h"

#include "spi/Assert.h"

namespace WPS {
namespace SPI {

size_t NMEAProtocol::tryParse(const char* data, size_t size)
{
    std::string nmeaBuffer(data, size);

    unsigned int parsedSentences = NMEA::ALL;
    const char* from = nmeaBuffer.c_str();

    size_t bytesParsed = NMEA::parse(from, _info, parsedSentences);
    if (bytesParsed > 0)
    {
        assert(bytesParsed > 2);

        updateGPSData();
        if (_logger.isDebugEnabled())
            _logger.debug("%.*s", (int) (bytesParsed - 2), from);

        if (parsedSentences & NMEA::GSV)
        {
            _gsvTimer.reset();
        }
        else if (_gsvTimer.elapsed() > GSV_TIMEOUT)
        {
            _info.clearSatsInView();
            _gsvTimer.reset();
        }
    }

    return bytesParsed;
}

void NMEAProtocol::updateGPSData()
{
    std::vector<GPSData::Satellite> satellites;

    Time time(0);

    if (_info.isPresent(NMEA::TIME) &&
        _info.isPresent(NMEA::DATE))
    {
        const Date date = { _info.getTime().hsecond * 10,
                            _info.getTime().second,
                            _info.getTime().minute,
                            _info.getTime().hour,
                            _info.getDate().day,
                            _info.getDate().month - 1,
                            _info.getDate().year + 100 };
        time = Time(date);
    }
    else
        time = Time::now();

    NMEA::SatsInView satsInView;
    _info.getSatsInView(satsInView);

    for (NMEA::SatsInView::const_iterator it = satsInView.begin();
         it != satsInView.end();
         ++it)
    {
        GPSData::Satellite satellite;
        satellite.satelliteId = it->prn;
        satellite.azimuth = it->azimuth;
        satellite.elevation = it->elevation;
        satellite.snr = it->snr;
        satellite.timetag = time.sec();
        satellites.push_back(satellite);
    }

    if (_info.getFixQuality() != NMEA::FIX_QUALITY_BAD)
    {
        GPSData::Fix fix;

        fix.quality = _info.getFixQuality();
        fix.latitude = NMEA::ndeg2degree(_info.getLatitude());
        fix.longitude = NMEA::ndeg2degree(_info.getLongitude());

        if (_info.isPresent(NMEA::ALTITUDE))
        {
            // height above MSL
            fix.altitude = _info.getAltitude();

            // height above WGS84 ellipsoid
            if (_info.isPresent(NMEA::DIFF))
                fix.height = fix.altitude + _info.getDiff();
        }

        fix.hdop = static_cast<float>(_info.getHDOP());
        fix.speed = _info.getSpeed() / 3.6f;
        fix.bearing = _info.getDirection();
        fix.svInFix = _info.getSatsInUse();
        _info.getSatsInUse(fix.prn);
        fix.gpsTime = time;
        fix.timetag = time.sec();

        _data = GPSData(fix, satellites);
    }
    else
        _data = GPSData(satellites);
}

}
}
