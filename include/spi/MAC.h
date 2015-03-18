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

#ifndef WPS_SPI_MAC_H_
#define WPS_SPI_MAC_H_

#include <string>

namespace WPS {
namespace SPI {

/**
 * \ingroup nonreplaceable
 *
 * Encapsulate a media access control (MAC) address
 *
 * @see http://en.wikipedia.org/wiki/MAC_address
 *
 * @author Skyhook Wireless
 */
class MAC
{
public:

    /**
     * A <i>binary</i> (48 bits) representation of a MAC.
     *
     * @note least significant byte first
     */
    typedef unsigned char raw_type[6];

    /**
     * A reference to a <i>binary</i> representation of a MAC.
     */
    typedef raw_type& raw_ref;

    /**
     * A const reference to a <i>binary</i> representation of a MAC.
     */
    typedef const raw_type& const_raw_ref;

    /**
     * Create a new MAC from a <i>binary</i> representation.
     *
     * @param mac the mac address.
     */
    MAC(const_raw_ref mac);

    MAC(const MAC& mac);
    MAC();

    MAC& operator=(const_raw_ref mac);
    MAC& operator=(const MAC& that);

    bool operator==(const MAC& that) const;
    bool operator==(const_raw_ref mac) const;
    bool operator!=(const MAC& that) const;
    bool operator<(const MAC& that) const;

    int compare(const MAC& that) const;

    /**
     * @return <code>this</code> MAC address as a 12-character hex string.
     */
    std::string toString() const;

    unsigned long long toLong() const;

    const_raw_ref getData() const;

    /**
     * Copy <code>this</code> MAC address to the <code>to</code> MAC address
     *
     * @param to the MAC address to copy to
     */
    void copy(raw_ref to) const;

    bool isGloballyUnique() const;

private:

    unsigned int getOUI() const;

private:

    unsigned char _mac[6];
};

}
}

#endif
