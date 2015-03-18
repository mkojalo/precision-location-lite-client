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

#ifndef WPS_SPI_LIBXMLDOM_H_
#define WPS_SPI_LIBXMLDOM_H_

#include "spi/DOM.h"

#include <libxml/tree.h>

namespace WPS {
namespace SPI {

/*********************************************************************/
/*                                                                   */
/* LibxmlDOMNode                                                     */
/*                                                                   */
/*********************************************************************/

class LibxmlDOMNode
    : public DOMNode
{
public:

    LibxmlDOMNode(xmlNode* node);

    std::string getNodeName() const;
    std::string getNodeValue() const;
    std::string getNamespaceURI() const;
    std::string getPrefix() const;
    std::string getLocalName() const;

    DOMNodeList* getChildNodes() const;

    std::string getAttributeNS(const std::string& namespaceURI,
                               const std::string& localName) const;

    DOMNode* getAttributeNodeNS(const std::string& namespaceURI,
                                const std::string& localName) const;

private:

    xmlNode* _p_node;
};

/*********************************************************************/
/*                                                                   */
/* LibxmlDOMNodeList                                                 */
/*                                                                   */
/*********************************************************************/

class LibxmlDOMNodeList
    : public DOMNodeList
{
public:

    LibxmlDOMNodeList(xmlNode* nodeList);

    DOMNode* getItem(unsigned long index) const;

    unsigned long getLength() const;

private:

    // libxml has no functional list support, it's just an old-fashioned
    // "struct with a pointer to next" type of list.
    xmlNode* _p_nodeList;
};

/*********************************************************************/
/*                                                                   */
/* LibxmlDOMDocument                                                 */
/*                                                                   */
/*********************************************************************/

class LibxmlDOMDocument
    : public DOMDocument
{
public:

    LibxmlDOMDocument(xmlDoc* p_doc);
    ~LibxmlDOMDocument();
    
    DOMNode* getDocumentElement() const;

private:

    xmlDoc* _p_doc;
};

}
}

#endif
