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

#include "../serial/SerialGPSAdapter.h"
#include "../protocol/GPSProtocol.h"

#include <stdlib.h>

#include "spi/Assert.h"

namespace {

const char* getDevice()
{
#ifdef WPS_SPI_GPS_DEVICE
    return WPS_SPI_GPS_DEVICE;
#else
    return getenv("WPS_SPI_GPS_DEVICE");
#endif
}

const char* getProtocol()
{
#ifdef WPS_SPI_GPS_PROTOCOL
    return WPS_SPI_GPS_PROTOCOL;
#else
    return getenv("WPS_SPI_GPS_PROTOCOL");
#endif
}

}

namespace WPS {
namespace SPI {

GPSAdapter*
GPSAdapter::newInstance()
{
    const char* deviceName = getDevice();
    const char* protocolName = getProtocol();

    if (! deviceName || ! protocolName)
        return NULL;

    SerialPort* const port = SerialPort::getById(deviceName);
    if (! port)
        return NULL;

    GPSProtocol* const protocol = GPSProtocol::newInstance(protocolName);
    if (! protocol)
        return NULL;

    return new SerialGPSAdapter(port, protocol);
}

}
}
