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

#pragma once

#include "../GPSProtocol.h"

#include "spi/Time.h"

#include "nmea/NMEA.h"
#include "nmea/Info.h"
#include "nmea/Units.h"

#include <string>

namespace WPS {
namespace SPI {

class NMEAProtocol
    : public GPSProtocol
{
public:

    NMEAProtocol()
    {}

    virtual size_t tryParse(const char* data, size_t size);

    virtual const char* id() const
    {
        return "nmea";
    }

    virtual void reset()
    {
        GPSProtocol::reset();
        _gsvTimer.reset();
        _info = NMEA::Info();
    }

private:

    void updateGPSData();

private:

    static const unsigned long GSV_TIMEOUT = 5000; // 5 sec

    Timer _gsvTimer;
    NMEA::Info _info;
};

}
}
