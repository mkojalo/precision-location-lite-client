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

#include "spi/WifiAdapter.h"
#include "spi/Time.h"
#include "spi/Assert.h"

namespace WPS {
namespace SPI {
    
/*********************************************************************/
/*                                                                   */
/* StaticWifiAdapter                                                 */
/*                                                                   */
/* NOTE: this adapter returns static data and is for testing         */
/*       purposes only                                               */
/*                                                                   */
/*********************************************************************/

class StaticWifiAdapter
    : public WifiAdapter
{
public:

    StaticWifiAdapter()
        : _listener(NULL)
    {}

    void setListener(Listener* listener)
    {
        _listener = listener;
    }

    std::string description() const
    {
        return "StaticWifiAdapter";
    }

    ErrorCode open()
    {
        return SPI_OK;
    }

    void close()
    {}

    void startScan()
    {
        assert(_listener != NULL);

        const MAC::raw_type macs[] =
              { { 0xF0, 0x17, 0xC9, 0x5B, 0x09, 0x00 },
                { 0x4E, 0x4D, 0xEC, 0x5B, 0x09, 0x00 },
                { 0x2A, 0x06, 0x82, 0x66, 0x0F, 0x00 } };

        const Timer now;

        const std::string ssidString("static");
        const ScannedAccessPoint::SSID ssid(ssidString.begin(), ssidString.end());

        std::vector<ScannedAccessPoint> scan;
        for (size_t i = 0; i < sizeof(macs) / sizeof(MAC::raw_type); ++i)
            scan.push_back(ScannedAccessPoint(MAC(macs[i]), -21, now, ssid));

        _listener->onScanCompleted(scan);
    }

    ErrorCode getConnectedMAC(MAC& mac)
    {
        return SPI_ERROR_NOT_READY;
    }

    ErrorCode getHardwareMAC(MAC& mac)
    {
        const MAC::raw_type hardwareMac = { 0x55, 0x44, 0x33, 0x22, 0x11, 0x00 };

        mac = hardwareMac;
        return SPI_OK;
    }

    ErrorCode power(PowerState)
    {
        return SPI_OK;
    }

private:

    Listener* _listener;
};

/*********************************************************************/
/*                                                                   */
/* WifiAdapter::newInstance                                          */
/*                                                                   */
/*********************************************************************/

WifiAdapter*
WifiAdapter::newInstance()
{
    return new StaticWifiAdapter;
}

}
}
