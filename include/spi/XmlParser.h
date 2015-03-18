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

#ifndef WPS_SPI_XMLPARSER_H_
#define WPS_SPI_XMLPARSER_H_

#include "spi/DOM.h"

namespace WPS {
namespace SPI {

/**
 * \addtogroup replaceable
 */
/** @{ */

/**
 * @author Skyhook Wireless
 */
class XmlParser
{
public:

    static XmlParser* newInstance();

    virtual ~XmlParser()
    {}

    virtual DOMDocument* parse(const char* buffer, std::size_t size) =0;

protected:

    XmlParser()
    {}

private:

    /**
     * XmlParser instances themselves cannot be copied.
     * Implementations may support copying.
     */
    XmlParser(const XmlParser&);
    XmlParser& operator=(const XmlParser&);
};

/** @} */

}
}

#endif
