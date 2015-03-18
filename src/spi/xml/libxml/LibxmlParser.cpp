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

#include "LibxmlDOM.h"
#include "LibxmlParser.h"

#include <libxml/parser.h>

namespace WPS {
namespace SPI {

/*********************************************************************
 *
 * LibxmlParser
 *
 *********************************************************************/

LibxmlParser::LibxmlParser()
    : _logger("WPS.SPI.XmlParser.LibxmlParser")
{}

LibxmlParser::~LibxmlParser()
{}

DOMDocument*
LibxmlParser::parse(const char* xml, size_t size)
{
    // Note: using the DOM-ish interface in libxml gives us no way
    // to intercept error reporting, which just dumps on stderr.
    // We'd have to switch to full-SAX parsing to get control of
    // error messages. Hence, we suppress those errors on stderr.
    static const int options
        = XML_PARSE_NOERROR | XML_PARSE_NOWARNING | XML_PARSE_PEDANTIC;

    xmlDoc* doc = xmlReadMemory(xml, size, "", NULL, options);
    if (! doc)
    {
        _logger.error("error parsing xml");
        return NULL;
    }

    return new LibxmlDOMDocument(doc);
}

/*********************************************************************
 *
 * XmlParser::newInstance
 *
 *********************************************************************/

XmlParser*
XmlParser::newInstance()
{
    return new LibxmlParser();
}

}
}
