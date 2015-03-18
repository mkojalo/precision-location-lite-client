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

#ifndef WPS_SPI_SYSTEM_INFORMATION_H_
#define WPS_SPI_SYSTEM_INFORMATION_H_

#include "spi/ErrorCodes.h"

#include <string>

namespace WPS {
namespace SPI {

/**
 * \addtogroup replaceable
 *
 * \b SystemInformation
 * \li \ref SystemInformation.h
 */
/** @{ */

/**
 * Information about target system.
 *
 * @since SHLC 1.0
 *
 * @author Skyhook Wireless
 */
class SystemInformation
{
public:

    /**
     * Operating system information.
     */
    struct OSInfo
    {
        /**
         * <code>winnt</code>, <code>macos</code> etc.
         * or empty if not available.
         */
        std::string type;

        /**
         * Version string for this OS/kernel/distribution
         * or empty if not available.
         */
        std::string version;
    };

    /**
     * Device information.
     */
    struct DeviceInfo
    {
        /**
         * Manufacturer name of this device (or PC motherboard)
         * or empty if not available.
         *
         * @note Must contain ASCII characters only.
         */
        std::string manufacturer;

        /**
         * Model string of this device (or PC motherboard)
         * or empty if not available.
         *
         * @note Must contain ASCII characters only.
         */
        std::string model;
    };

public:

    /**
     * @return a new instance.
     */
    static SystemInformation* newInstance();

    virtual ~SystemInformation()
    {}

    /**
     * Retrieve OS information.
     */
    virtual ErrorCode getOSInfo(OSInfo& info) =0;

    /**
     * Retrieve device information.
     */
    virtual ErrorCode getDeviceInfo(DeviceInfo& info) =0;

protected:

    SystemInformation()
    {}

private:

    /**
     * SystemInformation instances themselves cannot be copied.
     * Implementations may support copying.
     */
    SystemInformation(const SystemInformation&);
    SystemInformation& operator=(const SystemInformation&);
};

/** @} */

}
}

#endif
