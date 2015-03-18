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

#ifndef WPS_SPI_ASSERT_H_
#define WPS_SPI_ASSERT_H_

/*
 * ALWAYS INCLUDE THIS FILE LAST
 * OTHERWISE assert MAY BE REDEFINED TO ITS DEFAULT VALUE
 */

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <cstdarg>

#if ! defined(NDEBUG) && ! defined(WPS_NO_ASSERT)

namespace WPS {
namespace SPI {

// Implementation must avoid calling new operator or other SPI
// facilities that could call it (e.g. Logger)
void
_wps_assert_log(const char* format, ...);

/**
 * _wps_assert() implements \c assert() failures. 
 *
 * @see assert()
 */
void
_wps_assert(const char* file, unsigned lineno, const char* exp);

}  // namespace SPI
}  // namespace WPS

#  undef assert
#  if defined(__GNUC__)
#    define assert(e) \
       (__builtin_expect(!(e), 0) ? WPS::SPI::_wps_assert(__FILE__, __LINE__, #e) : (void) 0)
#  else
#    define assert(e) \
       ((e) ? (void) 0 : WPS::SPI::_wps_assert(__FILE__, __LINE__, #e))
#  endif

#endif  // NDEBUG
#endif  // WPS_SPI_ASSERT_H_
