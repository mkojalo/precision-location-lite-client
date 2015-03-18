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

#include "GPSProtocol.h"

#ifdef GPS_PROTOCOL_NMEA
#  include "nmea/NMEAProtocol.h"
#endif

#ifdef GPS_PROTOCOL_SIRF
#  include "sirf/SirfProtocol.h"
#endif

namespace WPS {
namespace SPI {

GPSProtocol* GPSProtocol::newInstance(const std::string& id)
{
#ifdef GPS_PROTOCOL_NMEA
    if (id == "nmea")
        return new NMEAProtocol();
#endif

#ifdef GPS_PROTOCOL_SIRF
    if (id == "sirf")
        return new SirfProtocol();
#endif

    return NULL;
}

std::list<GPSProtocol*> GPSProtocol::getSupportedProtocols()
{
    std::list<GPSProtocol*> result;

#ifdef GPS_PROTOCOL_NMEA
    result.push_back(new NMEAProtocol());
#endif

#ifdef GPS_PROTOCOL_SIRF
    result.push_back(new SirfProtocol());
#endif

    return result;
}

bool GPSProtocol::parse(const char* data, size_t size)
{
    _parseBuffer.append(data, size);

    size_t bytesParsed = tryParse(_parseBuffer.c_str(), _parseBuffer.size());

    if (bytesParsed > 0)
    {
        _state = GPSProtocol::OK;
        _parseBuffer.erase(0, bytesParsed);
    }
    else if (_parseBuffer.size() >= MAX_BUF_SIZE)
    {
        _logger.error("data stream seems to be broken");
        _state = GPSProtocol::FAILURE;

#ifndef NDEBUG
        if (_logger.isDebugEnabled())
            _logger.debug("ignoring %zu bytes of garbage: %s",
                          _parseBuffer.size(),
                          _parseBuffer.c_str());
#endif

        _parseBuffer.clear();
    }

    return _state != GPSProtocol::FAILURE;
}

}
}
