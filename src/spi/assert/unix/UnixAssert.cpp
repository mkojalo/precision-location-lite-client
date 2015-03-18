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

#include "spi/Assert.h"

namespace WPS {
namespace SPI {

#if ! defined(NDEBUG) && ! defined(WPS_NO_ASSERT)

void
_wps_assert_log(const char* format, ...)
{
    std::va_list args;
    va_start(args, format);

    puts("");
    ::vprintf(format, args);
    puts("");

    va_end(args);
}

void
_wps_assert(const char* file, unsigned lineno, const char* exp)
{
    _wps_assert_log("%s:%u@%s assertion failed", exp, lineno, file);
    __assert(exp, file, lineno);
}

#endif // NDEBUG

}
}
