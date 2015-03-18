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

#include <stdio.h>
#include <stdlib.h>

#ifdef HAVE_XLOCALE
#  include <xlocale.h>
#endif

#ifdef HAVE_STD_LOCALE
#  include <iostream>
#  include <sstream>
#  include <locale>
#endif

#if !defined HAVE_XLOCALE && !defined HAVE_STD_LOCALE
#  warning No xlocale or std locale is available, defaulting to no locale
#endif

#include "spi/StdLibC.h"
#include "spi/Assert.h"

namespace WPS {
namespace SPI {

#ifdef HAVE_XLOCALE

    static locale_t
    posix()
    {
        static locale_t posix = newlocale(LC_NUMERIC_MASK, "POSIX", NULL);
        assert(posix != NULL);
        return posix;
    }

#endif

#ifdef HAVE_STD_LOCALE

    template <typename T>
    T atox(const std::string& s)
    {
        std::istringstream iss(s);
        try
        {
            iss.imbue(std::locale("POSIX"));
        }
        catch (const std::exception& e)
        {
            std::cerr << e.what() << std::endl;
        }

        T t = 0;
        iss >> t;
        return t;
    }

    template <typename T>
    std::string xtoa(const T value, const int radix = 10)
    {
        std::ostringstream oss;
        try
        {
            oss.imbue(std::locale("POSIX"));
        }
        catch (const std::exception& e)
        {
            std::cerr << e.what() << std::endl;
        }

        if (radix == 8)
            oss << std::oct << value;
        else if (radix == 10)
            oss << value;
        else
            oss << std::hex << value;

        return oss.str();
    }

#endif

int vsnprintf(char* dest, size_t size, const char* format, std::va_list ap)
{
#ifdef PRINTF_SIZE_T_FORMAT_BROKEN
    // For platforms that don't support %z replace it with %l
    std::string finalFormat = format;
    const char* to_replace = "%z";
    for(size_t i = finalFormat.find(to_replace, 0);
        i != std::string::npos;
        i = finalFormat.find(to_replace, i))
    {
        finalFormat[i+1] = 'l';
    }

    format = finalFormat.c_str();
#endif

#if defined HAVE_XLOCALE && defined HAVE_PRINTF_L
    return ::vsnprintf_l(dest, size, posix(), format, ap);
#elif defined HAVE_XLOCALE && defined HAVE_STD_LOCALE
    locale_t previous = uselocale(posix());
    int rc = ::vsnprintf(dest, size, format, ap);
    uselocale(previous);
    return rc;
#else
    return ::vsnprintf(dest, size, format, ap);
#endif
}

long atol(const std::string& s)
{
#if defined HAVE_XLOCALE && defined HAVE_PRINTF_L
    return ::atol_l(s.c_str(), posix());
#elif defined HAVE_STD_LOCALE
    return atox<long>(s);
#else
    return ::atol(s.c_str());
#endif
}

double atof(const std::string& s)
{
#if defined HAVE_XLOCALE && defined HAVE_PRINTF_L
    return ::atof_l(s.c_str(), posix());
#elif defined HAVE_STD_LOCALE
    return atox<double>(s);
#else
    return ::atof(s.c_str());
#endif
}

unsigned long long strtoull(const std::string& s, int base)
{
#if defined HAVE_XLOCALE && defined HAVE_PRINTF_L
    return ::strtoull_l(s.c_str(), NULL, base, posix());
#elif defined HAVE_STD_LOCALE
    return atox<unsigned long long>(s);
#else
    return ::strtoull(s.c_str(), NULL, base);
#endif
}

std::string ltoa(long value, int radix)
{
    assert(radix == 8 || radix == 10 || radix == 16);

#ifdef HAVE_STD_LOCALE
    return xtoa<long>(value, radix);
#else
    const char* format;
    if (radix == 8)
        format = "%lo";
    else if (radix == 10)
        format = "%ld";
    else
        format = "%lx";

    char s[sizeof(value) * 8 + 1];
#  if defined HAVE_XLOCALE && defined HAVE_PRINTF_L
    int n = ::snprintf_l(s, sizeof s, posix(), format, value);
#  else
    int n = ::snprintf(s, sizeof s, format, value);
#  endif
    return std::string(s, n);
#endif
}

}
}
