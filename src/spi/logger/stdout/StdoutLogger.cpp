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
#include "spi/Time.h"
#include "spi/Thread.h"

#include <cstdio>
#include <string>

namespace WPS {
namespace SPI {

///////////////////////////////////////////////////////////////////////
// StdoutLogger
///////////////////////////////////////////////////////////////////////

class StdoutLogger
    : public Logger::Impl
{
public:

    StdoutLogger()
    {}

    bool isEnabledFor(const char* category, Logger::Level level) const
    {
#ifndef NDEBUG
        return true;
#elif defined WPS_MAX_LOG_LEVEL
        return level <= WPS_MAX_LOG_LEVEL;
#else
        return level <= Logger::LEVEL_INFO;
#endif
    }

    void log(const char* category,
             Logger::Level level,
             const char* fmt,
             std::va_list ap)
    {
        std::printf("%ld %lu %s [%s] %s\n",
                    timer.elapsed(),
                    Thread::id(),
                    category,
                    toString(level),
                    format(fmt, ap).c_str());
        std::fflush(stdout);
    }

private:

    StdoutLogger(const StdoutLogger&);
    StdoutLogger& operator=(const StdoutLogger&);

private:

    static const Timer timer;
};

const Timer StdoutLogger::timer;

///////////////////////////////////////////////////////////////////////
//                                                                   //
// Logger::Impl::getInstance                                         //
//                                                                   //
///////////////////////////////////////////////////////////////////////

Logger::Impl&
Logger::Impl::getInstance()
{
    static StdoutLogger logger;
    return logger;
}

}
}
