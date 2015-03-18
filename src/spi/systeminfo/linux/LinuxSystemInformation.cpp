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

#include "spi/SystemInformation.h"
#include "spi/Logger.h"

#include <fstream>
#include <iostream>
#include <algorithm>
#include <functional>

namespace {

// Parsing routines for the os-release/lsb-release key-value format.
// See:
//   http://0pointer.de/blog/projects/os-release
//   http://www.freedesktop.org/software/systemd/man/os-release.html

#define NOT_SPACE std::not1(std::ptr_fun<int, int>(std::isspace))

inline std::string
ltrim(const std::string& s)
{
    return std::string(std::find_if(s.begin(), s.end(), NOT_SPACE), s.end());
}

inline std::string
rtrim(const std::string& s)
{
    return std::string(s.begin(), std::find_if(s.rbegin(), s.rend(), NOT_SPACE).base());
}

inline std::string
trim(const std::string& s)
{
    return ltrim(rtrim(s));
}

inline bool
isQuote(const char c)
{
    return c == '\'' || c == '\"';
}

inline std::string
unquote(const std::string& s)
{
    const size_t len = s.length();
    if (len >= 2 && isQuote(s[0]) && isQuote(s[len-1]))
        return s.substr(1, len-2);
    else
        return s;
}

inline std::string
unescape(const std::string& s)
{
    std::string r(s);
    r.erase(std::remove(r.begin(), r.end(), '\\'), r.end());
    return r;
}

bool
parseKeyValue(const std::string& s,
              std::string& key,
              std::string& value)
{
    const size_t delimiter = s.find('=');
    if (delimiter == std::string::npos)
        return false;

    key = trim(s.substr(0, delimiter));
    if (key.empty() || key[0] == '#')  // # is a comment
        return false;

    value = unescape(unquote(trim(s.substr(delimiter + 1))));
    return true;
}

}  // anonymous namespace

namespace WPS {
namespace SPI {

class LinuxSystemInformation
    : public SystemInformation
{
public:

    LinuxSystemInformation()
        : _logger("WPS.SPI.LinuxSystemInformation")
    {}

    ErrorCode getOSInfo(OSInfo& info)
    {
        std::fstream fs("/etc/os-release", std::ios::in);
        if (! fs)
        {
            _logger.error("failed to open os-release file");
            return SPI_ERROR;
        }

        std::string line;
        while (std::getline(fs, line))
        {
            std::string key, value;
            if (parseKeyValue(line, key, value))
            {
                if (key == "NAME")
                    info.type = value;
                else if (key == "VERSION_ID")
                    info.version = value;
            }
        }

        if (info.type.empty())
        {
            _logger.error("couldn't find NAME in os-release file");
            return SPI_ERROR;
        }

        if (_logger.isDebugEnabled())
            _logger.debug("retrieved OS info: %s %s",
                          info.type.c_str(),
                          info.version.c_str());
        return SPI_OK;
    }

    ErrorCode getDeviceInfo(DeviceInfo& info)
    {
        std::fstream manufacturer("/sys/class/dmi/id/sys_vendor", std::ios::in);
        std::fstream model("/sys/class/dmi/id/product_name", std::ios::in);

        if (! manufacturer || ! model)
        {
            _logger.error("failed to open sys_vendor or product_name file");
            return SPI_ERROR;
        }

        if (! std::getline(manufacturer, info.manufacturer)
                || ! std::getline(model, info.model))
        {
            _logger.error("error reading sys_vendor or product_name file");
            return SPI_ERROR;
        }

        if (_logger.isDebugEnabled())
            _logger.debug("retrieved device info: %s %s",
                          info.manufacturer.c_str(),
                          info.model.c_str());
        return SPI_OK;
    }

private:

    Logger _logger;
};

SystemInformation*
SystemInformation::newInstance()
{
    return new LinuxSystemInformation;
}

}
}

