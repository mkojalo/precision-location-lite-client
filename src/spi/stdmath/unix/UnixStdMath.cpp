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

#include <cmath>
#include <cstdlib>

/**
 * some cmath implementations (e.g. STLport) don't remove
 * the isnan and isinf macros and put them in std.
 * we need to use and undef these macros before including spi/StdMath.h
 */
static bool wps_isnan(double x)
{
#ifdef isnan
    return isnan(x);
#  undef isnan
#else
    return std::isnan(x);
#endif
}

static bool wps_isinf(double x)
{
#ifdef isinf
    return isinf(x);
#  undef isinf
#else
    return std::isinf(x);
#endif
}

#include "spi/StdMath.h"

namespace WPS {
namespace SPI {
namespace Math {

long abs(long x)
{
    return std::abs(x);
}

long long abs(long long x)
{
#if HAVE_LLABS
    return ::llabs(x);
#else
    return std::abs(x);
#endif
}

double fabs(double x)
{
    return std::fabs(x);
}

double ceil(double x)
{
    return std::ceil(x);
}

double log(double x)
{
    return std::log(x);
}

double log10(double x)
{
    return std::log10(x);
}

double pow(double x, double y)
{
    return std::pow(x, y);
}

double sqrt(double x)
{
    return std::sqrt(x);
}

double sin(double x)
{
    return std::sin(x);
}

double cos(double x)
{
    return std::cos(x);
}

double atan(double x)
{
    return std::atan(x);
}

double atan2(double x, double y)
{
    return std::atan2(x, y);
}

double asin(double x)
{
    return std::asin(x);
}

double acos(double x)
{
    return std::acos(x);
}

bool isnan(double x)
{
    return wps_isnan(x);
}

bool isinf(double x)
{
    return wps_isinf(x);
}

double modf(double x, double* intpart)
{
    return std::modf(x, intpart);
}

}
}
}
