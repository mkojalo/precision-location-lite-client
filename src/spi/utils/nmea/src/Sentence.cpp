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

#include "Sentence.h"
#include "spi/StdLibC.h"

#include <algorithm>

#include "spi/Assert.h"

namespace WPS {
namespace SPI {
namespace NMEA {

static int
hexToNumber(char h)
{
    if (h >= '0' && h <= '9')
        return h - '0';

    if (h >= 'A' && h <= 'F')
        return h - 'A' + 0xA;

    return h - 'a' + 0xA;
}

inline int
extractChecksum(const char* from)
{
    return (hexToNumber(from[0]) << 4) | hexToNumber(from[1]);
}

static int
calcChecksum(const char* payload, size_t length)
{
    int checksum = 0;

    for (size_t i = 0; i < length; ++i)
        checksum ^= static_cast<int>(payload[i]);

    return checksum;
}

static const char*
findChar(const char* from, char c, size_t length)
{
    const char* p = std::find(from, from + length, c);
    if (p == from + length)
        return NULL;
    else
        return p;
}

static size_t
calcTokens(const char* payload, size_t length)
{
    const char* p = payload;
    size_t n = 1;
    size_t offset;

    while ((offset = p - payload) < length
           && (p = findChar(p, ',', length - offset)))
    {
        ++n;
        ++p;
    }

    return n;
}

void
Sentence::toString(const Dataset& from, std::string& to) const
{
    std::string s(_head);

    for (Format::const_iterator it = _format.begin();
         it != _format.end();
         ++it)
    {
        s += ',';
        if (from.isPresent(it->first))
            it->second->toString(from.get(it->first), s);
    }

    char checksum[3];
    snprintf(checksum, sizeof checksum, "%02X", calcChecksum(s.c_str(), s.length()));

    to += '$';
    to += s;
    to += '*';
    to += checksum;
    to += "\r\n";
}

bool
Sentence::parse(const char* from, size_t length, Dataset& to) const
{
    if (length < _head.length() + 4) // must not be shorter than 'GPXXX,*AA'
        return false;

    if (_head.compare(0, _head.length(), from, 0, _head.length()) != 0)
        return false;

    const size_t checksumOffset = length - 3;

    if (calcChecksum(from, checksumOffset) != extractChecksum(&from[checksumOffset + 1]))
        return false;

    return parsePayload(&from[_head.length() + 1],
                        checksumOffset - _head.length() - 1,
                        to);
}

bool
Sentence::parsePayload(const char* from, size_t length, Dataset& to) const
{
    // validate number of tokens to detect wrong message format (see GSA1)
    // and avoid parsing portion of data that might mismatch the current format
    if (calcTokens(from, length) != _format.size())
        return false;

    const char* p = from;

    for (Format::const_iterator it = _format.begin();
         it != _format.end();
         ++it)
    {
        const size_t offset = p - from;
        assert(offset <= length);

        const size_t remaining = length - offset;

        const char* pComma = findChar(p, ',', remaining);
        const size_t tokenLength = pComma ? pComma - p : remaining;

        Variant value;
        if (tokenLength > 0
            && it->second->parse(p, tokenLength, value))
        {
            to.set(it->first, value);
        }
        else
            to.remove(it->first);

        p += tokenLength + 1;
    }

    return true;
}

const char*
Sentence::find(const char* from, size_t& length, const char*& next)
{
    const char* p;
    for (p = from; p[0]; ++p)
        if (p[0] == '$')
            break;

    if (!p[0])
        return NULL;

    const char* begin = ++p;

    for (; p[0]; ++p)
        if (p[0] == '\r' && p[1] == '\n')
            break;

    if (!p[0])
        return NULL;

    length = p - begin;
    next = p + 2;
    return begin;
}

}
}
}
