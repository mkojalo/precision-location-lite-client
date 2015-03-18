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

#include "spi/GPSData.h"
#include "spi/Logger.h"

#include <list>

namespace WPS {
namespace SPI {

class GPSProtocol
{
public:

    enum State
    {
        UNKNOWN,
        OK,
        FAILURE,
        HARDWARE_FAILURE
    };

    GPSProtocol()
        : _state(UNKNOWN),
          _logger("WPS.SPI.GPSProtocol")
    {
        _parseBuffer.reserve(MAX_BUF_SIZE);
    }

    virtual ~GPSProtocol()
    {}

    bool parse(const char* data, size_t size);
    virtual const char* id() const = 0;

    const GPSData& data() const
    {
        return _data;
    }

    State state() const
    {
        return _state;
    }

    virtual void reset()
    {
        _data.clear();
        _state = UNKNOWN;
    }

    static GPSProtocol* newInstance(const std::string& id);
    static std::list<GPSProtocol*> getSupportedProtocols();

protected:

    virtual size_t tryParse(const char* data, size_t size) = 0;

    static const size_t MAX_BUF_SIZE = 1024;

    GPSData _data;
    State _state;
    Logger _logger;
    std::string _parseBuffer;
};

}
}
