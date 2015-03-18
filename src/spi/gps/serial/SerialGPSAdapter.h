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

#include "spi/GPSAdapter.h"
#include "spi/Concurrent.h"
#include "spi/StdLibC.h"

#include "SerialPort.h"

#include "protocol/GPSProtocol.h"

#include "spi/Assert.h"

namespace WPS {
namespace SPI {

class SerialGPSAdapter
    : public GPSAdapter
    , public SerialPort::Listener
{
public:

    SerialGPSAdapter(SerialPort* port, GPSProtocol* protocol)
        : _port(port)
        , _protocol(protocol)
        , _numTimeouts(0)
    {
        _port->setTimeout(500);
    }

    ~SerialGPSAdapter()
    {
        close();
    }

    std::string description() const
    {
        char s[128];
        snprintf(s,
                 sizeof(s),
                 "SerialGPSAdapter (%s:%d, %s)",
                 _port->id().c_str(),
                 _port->getBaudRate(),
                 _protocol->id());
        return s;
    }

    void setListener(GPSAdapter::Listener* listener)
    {
        _listener = listener;
    }

    ErrorCode open()
    {
        assert(_listener != NULL);

        if (_port->start(this))
            return SPI_OK;
        else
            return SPI_ERROR;
    }

    void close()
    {
        _port->stop();
        _numTimeouts = 0;
    }

private:

    bool onData(SerialPort* port, const char* data, unsigned int size)
    {
        assert(_listener != NULL);

        _protocol->parse(data, size);
        _numTimeouts = 0;
        _listener->onGpsData(_protocol->data());
        return true;
    }

    bool onTimeout(SerialPort* port)
    {
        assert(_listener != NULL);

        if (++_numTimeouts >= 4)
            _listener->onGpsError(SPI_ERROR_IO);  // Got 500ms timeout 4 times, i.e. 2 seconds overall
        return true;
    }

    bool onError(SerialPort* port)
    {
        assert(_listener != NULL);
        _listener->onGpsError(SPI_ERROR);
        return false;
    }

private:

    std::auto_ptr<SerialPort> _port;
    std::auto_ptr<GPSProtocol> _protocol;
    GPSAdapter::Listener* _listener;
    unsigned _numTimeouts;
};

}
}
