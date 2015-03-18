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

#include "XmlUtils.h"

#include "spi/StdLibC.h"

#include <stdint.h>
#include <limits>

#include "spi/Assert.h"

namespace WPS {
namespace API {

/***************************************************************************/
/* xmlEscape                                                               */
/***************************************************************************/

std::string
xmlEscape(const std::string& s)
{
    std::string::const_iterator it = s.begin();
    for (; it != s.end(); ++it)
    {
        const unsigned char c = *it;
        assert(c != '\0');

        if (c == '&'
                || c == '"'
                || c == '\''
                || c == '<'
                || c == '>')
            break;
    }

    if (it == s.end())
        return s;

    std::string result;
    result.reserve(s.size());
    result.append(s.begin(), it);

    for (; it != s.end(); ++it)
    {
        const unsigned char c = *it;
        assert(c != '\0');

        switch (c)
        {
        case '&':
            result.append("&amp;");
            break;
        case '<':
            result.append("&lt;");
            break;
        case '>':
            result.append("&gt;");
            break;
        case '"':
            result.append("&quot;");
            break;
        case '\'':
            result.append("&apos;");
            break;
        default:
            result.append(1, c);
        }
    }

    return result;
}

/***************************************************************************/
/* xmlUtf8Test                                                             */
/***************************************************************************/

typedef std::pair<size_t, unsigned> Utf8Char;  // (length of utf-8 sequence, codepoint)

static Utf8Char
decodeUtf8FirstByte(const char c)
{
    // U+0000 - U+007F
    if ((c & 0x80) == 0)
        return Utf8Char(1, static_cast<unsigned char>(c));

    // U+0080 - U+07FF
    if ((c & 0xE0) == 0xC0)
        return Utf8Char(2, static_cast<unsigned char>(c & ~0xE0));

    // U+0800 - U+FFFF
    if ((c & 0xF0) == 0xE0)
        return Utf8Char(3, static_cast<unsigned char>(c & ~0xF0));

    // U+10000 - U+10FFFF
    if ((c & 0xF8) == 0xF0)
        return Utf8Char(4, static_cast<unsigned char>(c & ~0xF8));

    // invalid start byte
    return Utf8Char(0, 0);
}

static Utf8Char
decodeUtf8Char(std::vector<unsigned char>::const_iterator p,
               std::vector<unsigned char>::const_iterator end)
{
    Utf8Char c = decodeUtf8FirstByte(*p);
    if (! c.first)
        return c;

    if (static_cast<size_t>(std::distance(p, end)) < c.first)
        return Utf8Char(0, 0);  // not enough bytes in input

    for (size_t i = 1; i < c.first; ++i)
    {
        ++p;

        if ((*p & 0xC0) != 0x80)  
            return Utf8Char(0, 0);  // continuation byte expected

        c.second = (c.second << 6) | static_cast<unsigned char>(*p & (~0xC0));
    }

    return c;
}

static bool
isUtf8OverlongSequence(const Utf8Char& c)
{
    if (c.second <= 0x7F)
        return c.first != 1;
    if (c.second >= 0x80 && c.second <= 0x7FF)
        return c.first != 2;
    if (c.second >= 0x800 && c.second <= 0xFFFF)
        return c.first != 3;
    if (c.second >= 0x10000 && c.second <= 0x10FFFF)
        return c.first != 4;

    return true;
}

static bool
isValidXmlUtf8(const unsigned c)
{
    // http://en.wikipedia.org/wiki/Valid_characters_in_XML
    return c == 0x09
        || c == 0x0A
        || c == 0x0D
        || (c >= 0x20 && c <= 0xD7FF)
        || (c >= 0xE000 && c <= 0xFFFD)
        || (c >= 0x10000 && c <= 0x10FFFF);
}

bool
xmlUtf8Test(const std::vector<unsigned char>& v)
{
    for (std::vector<unsigned char>::const_iterator it = v.begin();
         it != v.end();
         ++it)
    {
        const Utf8Char c = decodeUtf8Char(it, v.end());
        if (isUtf8OverlongSequence(c) || ! isValidXmlUtf8(c.second))
            return false;

        assert(c.first > 0);
        it += c.first - 1;
    }

    return true;
}

}
}
