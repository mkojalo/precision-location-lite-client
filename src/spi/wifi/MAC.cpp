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

#include "spi/MAC.h"
#include "spi/StdLibC.h"
#include "spi/StdMath.h"

#include <stdio.h>
#include <string.h>

#if defined(ULONG_LONG_MAX) && ULONG_LONG_MAX < 0xFFFFFFFFFFFFULL
#  error "long long expected to be at least 6 bytes"
#endif

namespace WPS {
namespace SPI {

static const unsigned int virtualOUI[] = {
    0x080027,   // VirtualBox (Cadmus)
    0x005056,   // VMWare
    0x001C14,   // VMWare
    0x000C29,   // VMWare
    0x000569,   // VMWare
    0x001c42    // Parallels
};

MAC::MAC(MAC::const_raw_ref mac)
{
    memcpy(_mac, mac, sizeof _mac);
}

MAC::MAC(const MAC& mac)
{
    mac.copy(_mac);
}

MAC::MAC()
{
    memset(_mac, 0, sizeof _mac);
}

MAC&
MAC::operator=(const MAC& that)
{
    that.copy(_mac);
    return *this;
}

MAC&
MAC::operator=(const_raw_ref mac)
{
    memcpy(_mac, mac, sizeof _mac);
    return *this;
}

bool
MAC::operator==(const MAC& that) const
{
    return memcmp(_mac, that._mac, sizeof _mac) == 0;
}

bool
MAC::operator==(const_raw_ref mac) const
{
    return memcmp(_mac, mac, sizeof _mac) == 0;
}

bool
MAC::operator!=(const MAC& that) const
{
    return memcmp(_mac, that._mac, sizeof _mac) != 0;
}

bool
MAC::operator<(const MAC& that) const
{
    return compare(that) < 0;
}

int
MAC::compare(const MAC& that) const
{
    // This will sort MACs in a natural way:
    //   000000000001
    //   000000000002
    //   000000001000
    //   000000002000
    //   100000000000
    //   200000000000
    for (int i = sizeof(_mac) - 1; i >= 0; --i)
    {
        if (_mac[i] > that._mac[i])
            return 1;
        if (_mac[i] < that._mac[i])
            return -1;
    }
    return 0;
}

std::string
MAC::toString() const
{
    char buf[13];
    snprintf(buf,
             sizeof(buf),
             "%02X%02X%02X%02X%02X%02X",
             (unsigned long) _mac[5],
             (unsigned long) _mac[4],
             (unsigned long) _mac[3],
             (unsigned long) _mac[2],
             (unsigned long) _mac[1],
             (unsigned long) _mac[0]);
    return buf;
}

unsigned long long
MAC::toLong() const
{
    unsigned long long n = 0;
#if (BYTE_ORDER == LITTLE_ENDIAN)
    memcpy(&n, getData(), sizeof(MAC::raw_type));
#else
    MAC::const_raw_ref raw = getData();
    std::reverse_copy(raw,
                      raw + sizeof(MAC::raw_type),
                      &n);
#endif
    return n;
}

MAC::const_raw_ref
MAC::getData() const
{
    return _mac;
}

void
MAC::copy(raw_ref to) const
{
    if (to != _mac)
        memcpy(to, _mac, sizeof _mac);
}

bool
MAC::isGloballyUnique() const
{
    if ((_mac[5] & 0x2) != 0)
        return false;

    const unsigned int oui = getOUI();
    const unsigned int* end = virtualOUI + sizeof(virtualOUI) / sizeof(virtualOUI[0]);

    return std::find(virtualOUI, end, oui) == end;
}

unsigned int
MAC::getOUI() const
{
    unsigned int n = 0;
#if (BYTE_ORDER == LITTLE_ENDIAN)
    memcpy(&n, getData() + 3, 3);
#else
    MAC::const_raw_ref raw = getData();
    std::reverse_copy(raw + 3, raw + 6, &n);
#endif
    return n;
}

}
}
