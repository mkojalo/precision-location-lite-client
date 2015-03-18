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

#include "spi/Logger.h"

namespace WPS {
namespace SPI {

///////////////////////////////////////////////////////////////////////
// NullLogger
///////////////////////////////////////////////////////////////////////

class NullLogger
    : public Logger::Impl
{
public:

    NullLogger()
    {}

    bool isEnabledFor(const char*, Logger::Level) const
    {
        return false;
    }

    void log(const char*, Logger::Level, const char*, std::va_list)
    {}

private:

    NullLogger(const NullLogger&);
    NullLogger& operator=(const NullLogger&);
};

///////////////////////////////////////////////////////////////////////
//                                                                   //
// Logger::Impl::getInstance                                         //
//                                                                   //
///////////////////////////////////////////////////////////////////////

Logger::Impl&
Logger::Impl::getInstance()
{
    static NullLogger instance;
    return instance;
}

}
}
