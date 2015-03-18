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

#ifndef WPS_SPI_STD_LIB_C_H
#define WPS_SPI_STD_LIB_C_H

#include <algorithm>
#include <cstdarg>
#include <string>

#include <stdlib.h>

#if HAVE_SYS_TYPES_H
#  include <sys/types.h>
#endif

namespace WPS {
namespace SPI {

/**
 * \addtogroup replaceable
 *
 * \b Standard -- standard (and additional non-standard) functions
 * \li \ref StdLibC.h
 * \li \ref StdMath.h
 */

/**
 * \ingroup replaceable
 *
 * Convert arguments to a formatted string in the en_US locale.
 *
 * @param dest the destination buffer
 * @param size the size of the destination buffer <code>dest</code>
 * @param format the format string
 * @param arg arguments to be formatted according to <code>format</code>
 *            \n
 *            Note that the state of <code>ap</code> is undefined
 *            upon return.
 *
 * @return the number of characters written to <code>dest</code>
 *         (not including the trailing null).
 *         No more than <code>size</code> characters are actually
 *         written to <code>dest</code>, so a return value
 *         of <code>size</code> or more indicates that the output
 *         was truncated.
 *
 * @see vsnprintf in ISO/IEC 9899:1999
 * @see vsnprintf_l in POSIX
 *
 * @author Skyhook Wireless
 */
int vsnprintf(char* dest, size_t size, const char* format, std::va_list arg);

/**
 * Convert arguments to a formatted string in the en_US locale.
 *
 * @param dest the destination buffer
 * @param size the size of the destination buffer <code>dest</code>
 * @param format the format string
 *
 * @return the number of characters written to <code>dest</code>
 *         (not including the trailing null).
 *         No more than <code>size</code> characters are actually
 *         written to <code>dest</code>, so a return value
 *         of <code>size</code> or more indicates that the output
 *         was truncated and the size of the output buffer should
 *         have been to not be truncated.
 *
 * @see snprintf in ISO/IEC 9899:1999
 *
 * @author Skyhook Wireless
 */
inline int snprintf(char* dest, size_t size, const char* format, ...)
{
    std::va_list ap;

    va_start(ap, format);
    const int ret = WPS::SPI::vsnprintf(dest, size, format, ap);
    va_end(ap);

    return ret;
}

/**
 * \ingroup replaceable
 *
 * Convert a character string, in the en_US locale, to a long.
 *
 * @param s string to be converted.
 *
 * @return the converted value
 *
 * @see atol in ISO/IEC 9899:1990
 *
 * @author Skyhook Wireless
 */
long atol(const std::string& s);

/**
 * Convert a character string, in the en_US locale, to an integer.
 *
 * @param s string to be converted
 *
 * @return the converted value
 *
 * @author Skyhook Wireless
 */
inline int atoi(const std::string& s)
{
    return atol(s);
}

/**
 * \ingroup replaceable
 *
 * Convert a character string, in the en_US locale, to a double.
 *
 * @param s the character string to convert
 *
 * @return the converted value
 *
 * @see atof in ISO/IEC 9899:1990
 *
 * @author Skyhook Wireless
 */
double atof(const std::string& s);

/**
 * \ingroup replaceable
 *
 * Convert a character string, in the en_US locale, to an
 * unsigned long long.
 *
 * @param s the character string to convert
 * @param base numerical base (radix) that determines the valid
 *             characters and their interpretation. If this is 0,
 *             the base used is determined by the format in the sequence
 *
 * @return the converted value
 *
 * @author Skyhook Wireless
 */
unsigned long long strtoull(const std::string& s, int base = 10);

/**
 * \ingroup replaceable
 *
 * Convert a long to a character string in the en_US locale.
 *
 * @param value value to be converted
 * @param radix radix in which value has to be represented,
 *              between <code>2</code> and <code>36</code>.
 *              \n
 *              If <code>radix</code> is <code>10</code>
 *              and <code>value</code> is negative
 *              the string is preceeded by the minus sign.
 *              With any other radix, <code>value</code>
 *              is always considered unsigned.
 *
 * @return the converted string
 *
 * @author Skyhook Wireless
 */
std::string ltoa(long value, int radix = 10);


/**
 * Convert an integer to a character string in the en_US locale.
 *
 * @param value value to be converted
 * @param radix radix in which value has to be represented,
 *              between <code>2</code> and <code>36</code>.
 *              \n
 *              If <code>radix</code> is <code>10</code>
 *              and <code>value</code> is negative
 *              the string is preceeded by the minus sign.
 *              With any other radix, <code>value</code>
 *              is always considered unsigned.
 *
 * @return the converted string
 *
 * @author Skyhook Wireless
 */
inline std::string itoa(int value, int radix = 10)
{
    return ltoa(value, radix);
}

/**
 * Copy block of memory.
 *
 * @param dest point to the destination block of memory.
 * @param src point to the source block of memory.
 * @param n number of bytes to copy.
 * @return <code>dest</code> is returned.
 *
 * @see memcpy in ISO/IEC 9899:1990 (ISO C90)
 *
 * @author Skyhook Wireless
 */
inline void* memcpy(void* dest, const void* src, size_t n)
{
    char* d = reinterpret_cast<char*>(dest);
    const char* s = reinterpret_cast<const char*>(src);
    std::copy(s, s + n, d);
    return dest;
}

/**
 * Fill block of memory.
 *
 * @param ptr point to the destination array.
 * @param value value to be set.
 * @param n number of bytes to set.
 * @return <code>ptr</code> is returned.
 *
 * @see memset in ISO/IEC 9899:1990 (ISO C90)
 *
 * @author Skyhook Wireless
 */
inline void* memset(void* ptr, int value, size_t n)
{
    std::fill_n(reinterpret_cast<char*>(ptr), n, value);
    return ptr;
}

/**
 * Compare two blocks of memory.
 *
 * @param ptr1 point to the first block of memory.
 * @param ptr2 point to the second block of memory.
 * @param n number of bytes to compare.
 * @return <code>true</code> if both blocks of memory are equal.
 *
 * @see memcmp in ISO/IEC 9899:1990 (ISO C90)
 *
 * @author Skyhook Wireless
 */
inline bool memeq(const void* ptr1, const void* ptr2, size_t n)
{
  const char* p1 = reinterpret_cast<const char*>(ptr1);
  const char* p2 = reinterpret_cast<const char*>(ptr2);
  return std::equal(p1, p1 + n, p2);
}

}
}

#endif
