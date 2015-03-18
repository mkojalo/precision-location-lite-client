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

#ifdef HAVE_ALLOCA_H
#  include <alloca.h>
#endif

#include "spi/StdLibC.h"

#ifndef va_copy
#   ifdef __va_copy
#       define va_copy(a, b) __va_copy(a, b)
#   else
#       define va_copy(a, b) ((a) = (b))
#   endif
#endif

#include "spi/Assert.h"

namespace WPS {
namespace SPI {

/*static*/ const char* Logger::Impl::toString(Level level)
{
    switch (level)
    {
    case Logger::LEVEL_OFF:
        return "OFF";
    case Logger::LEVEL_FATAL:
        return "FATAL";
    case Logger::LEVEL_ALERT:
        return "ALERT";
    case Logger::LEVEL_CRITICAL:
        return "CRITICAL";
    case Logger::LEVEL_ERROR:
        return "ERROR";
    case Logger::LEVEL_WARN:
        return "WARN";
    case Logger::LEVEL_NOTICE:
        return "NOTICE";
    case Logger::LEVEL_INFO:
        return "INFO";
    case Logger::LEVEL_DEBUG:
        return "DEBUG";
    case Logger::LEVEL_ON:
        return "ON";
    default:
        return "???";
    }
}

/*static*/ std::string Logger::Impl::format(const char* format, std::va_list ap)
{
    std::va_list ap2;
    va_copy(ap2, ap);

#ifdef HAVE_ALLOCA_H
    char buf[128];
#else
    char buf[512];
#endif
    size_t n = WPS::SPI::vsnprintf(buf, sizeof(buf), format, ap);
    assert(n >= 0);
    if (n < sizeof(buf))
    {
        va_end(ap2);
        return std::string(buf, n);
    }
#ifdef HAVE_ALLOCA_H
    char* buf2 = n < 4096 ? (char*) alloca(n+1) : new char[n+1];
#else
    char* buf2 = new char[n+1];
#endif
    size_t n2 = WPS::SPI::vsnprintf(buf2, n+1, format, ap2);
    assert(n == n2);
    std::string s(buf2, n2);
#ifdef HAVE_ALLOCA_H
    if (n >= 4096)
        delete[] buf2;
#else
    delete[] buf2;
#endif
    return s;
}

void Logger::log(Logger::Level level, const char* format, std::va_list args) const
{
    if (isEnabledFor(level))
        _impl.log(_category, level, format, args);
}

void Logger::log(Level level, const char* format, ...) const
{
    if (! isEnabledFor(level))
        return;
    std::va_list args;
    va_start(args, format);
    _impl.log(_category, level, format, args);
    va_end(args);
}

void Logger::fatal(const char* format, ...) const
{
    if (! isFatalEnabled())
        return;
    std::va_list args;
    va_start(args, format);
    _impl.log(_category, LEVEL_FATAL, format, args);
    va_end(args);
}

void Logger::alert(const char* format, ...) const
{
    if (! isAlertEnabled())
        return;
    std::va_list args;
    va_start(args, format);
    _impl.log(_category, LEVEL_ALERT, format, args);
    va_end(args);
}

void Logger::critical(const char* format, ...) const
{
    if (! isCriticalEnabled())
        return;
    std::va_list args;
    va_start(args, format);
    _impl.log(_category, LEVEL_CRITICAL, format, args);
    va_end(args);
}

void Logger::error(const char* format, ...) const
{
    if (! isErrorEnabled())
        return;
    std::va_list args;
    va_start(args, format);
    _impl.log(_category, LEVEL_ERROR, format, args);
    va_end(args);
}

void Logger::warn(const char* format, ...) const
{
    if (! isWarnEnabled())
        return;
    std::va_list args;
    va_start(args, format);
    _impl.log(_category, LEVEL_WARN, format, args);
    va_end(args);
}

void Logger::notice(const char* format, ...) const
{
    if (! isNoticeEnabled())
        return;
    std::va_list args;
    va_start(args, format);
    _impl.log(_category, LEVEL_NOTICE, format, args);
    va_end(args);
}

void Logger::info(const char* format, ...) const
{
    if (! isInfoEnabled())
        return;
    std::va_list args;
    va_start(args, format);
    _impl.log(_category, LEVEL_INFO, format, args);
    va_end(args);
}

void Logger::debug(const char* format, ...) const
{
    if (! isDebugEnabled())
        return;
    std::va_list args;
    va_start(args, format);
    _impl.log(_category, LEVEL_DEBUG, format, args);
    va_end(args);
}

}
}
