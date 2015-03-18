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

#if defined(__linux__) || defined(__APPLE__) || defined(__CYGWIN__)
#  include "iconvert.h"
#endif

#include <cwchar>
#include <memory>

namespace WPS {
namespace SPI {

#if defined(__linux__) || defined(__APPLE__) || defined(__CYGWIN__)

typedef std::basic_string<xmlChar> xstring;

static inline std::string
to_string(const xmlChar* p_xml)
{
    if (! p_xml)
        return "";
    return iconvert<xmlChar, char>("utf-8", "", p_xml, xmlStrlen(p_xml));
}

static inline xstring
to_xstring(const std::string& str)
{
    return iconvert<char, xmlChar>("", "utf-8", str.c_str(), str.length());
}

#else
#  error "Need implementations for to_string and to_xstring"
#endif

/*********************************************************************/
/*                                                                   */
/* LibxmlDOMNode                                                     */
/*                                                                   */
/*********************************************************************/

LibxmlDOMNode::LibxmlDOMNode(xmlNode* p_node)
    : _p_node(p_node)
{}

std::string
LibxmlDOMNode::getNodeName() const
{
    std::string name("");
    if (! _p_node)
        return name;

    // libxml ->name is always localname, must prepend ns if not null
    xmlNs* p_ns = _p_node->ns;
    if (p_ns && p_ns->prefix)
        name += to_string(p_ns->prefix) + ":";
    name += to_string(_p_node->name);
    return name;
}

std::string
LibxmlDOMNode::getNodeValue() const
{
    if (! _p_node)
        return "";

    if (_p_node->type == XML_ELEMENT_NODE)
        return to_string(_p_node->children ? _p_node->children->content : 0);

    if (_p_node->type == XML_ATTRIBUTE_NODE)
    {
        xmlAttr* p_attr = reinterpret_cast<xmlAttr*>(_p_node);
        return to_string(p_attr->children ? p_attr->children->content : 0);
    }

    if (_p_node->type == XML_TEXT_NODE)
        return to_string(_p_node->content);

    return "";
}

std::string
LibxmlDOMNode::getNamespaceURI() const
{
    xmlNs* p_ns = _p_node ? _p_node->ns : 0;
    return to_string(p_ns ? p_ns->href : 0);
}

std::string
LibxmlDOMNode::getPrefix() const
{
    xmlNs* p_ns = _p_node ? _p_node->ns : 0;
    return to_string(p_ns ? p_ns->prefix : 0);
}

std::string
LibxmlDOMNode::getLocalName() const
{
    // in libxml ->name is local
    return to_string(_p_node ? _p_node->name : 0);
}

DOMNodeList*
LibxmlDOMNode::getChildNodes() const
{
    return new LibxmlDOMNodeList(_p_node ? _p_node->children : 0);
}

std::string
LibxmlDOMNode::getAttributeNS(const std::string& namespaceURI,
                              const std::string& localName) const
{
    std::auto_ptr<DOMNode> attr(getAttributeNodeNS(namespaceURI, localName));
    return attr.get() != NULL ? attr->getNodeValue() : "";
}

// we use this because libxml will have a NULL ns string for default ns
static inline bool
nsEqual(const xmlChar* ns1, const xmlChar* ns2)
{
    // Note: We're careful not to deref a null ptr
    if (! ns1)
        return ! ns2 || ! *ns2;
    else if (! ns2)
        return ! *ns1;
    else
        return xmlStrEqual(ns1, ns2);
}

DOMNode*
LibxmlDOMNode::getAttributeNodeNS(const std::string& namespaceURI,
                                  const std::string& localName) const
{
    if (! _p_node || _p_node->type != XML_ELEMENT_NODE || ! _p_node->properties)
        return 0;

    const xstring uri = to_xstring(namespaceURI);
    const xstring local = to_xstring(localName);
    for (xmlAttr* iter = _p_node->properties; iter; iter = iter->next)
    {
        const xmlChar* ns = iter->ns ? iter->ns->href : 0;
        if ( nsEqual(ns, uri.c_str()) && xmlStrEqual(iter->name, local.c_str()))
            return new LibxmlDOMNode(reinterpret_cast<xmlNode*>(iter));
    }

    return 0;
}

/*********************************************************************/
/*                                                                   */
/* LibxmlDOMNodeList                                                 */
/*                                                                   */
/*********************************************************************/

LibxmlDOMNodeList::LibxmlDOMNodeList(xmlNode* nodeList)
    : _p_nodeList(nodeList)
{}

DOMNode*
LibxmlDOMNodeList::getItem(unsigned long index) const
{
    unsigned long i = 0;
    for (xmlNode* iter = _p_nodeList; iter;  iter = iter->next, ++i)
    {
        if (index == i)
            return new LibxmlDOMNode(iter);
    }

    return 0;
}

unsigned long
LibxmlDOMNodeList::getLength() const
{
    unsigned long len = 0;
    for (xmlNode* iter = _p_nodeList; iter; iter = iter->next)
        ++len;
    return len;
}

/*********************************************************************/
/*                                                                   */
/* LibxmlDOMDocument                                                 */
/*                                                                   */
/*********************************************************************/

LibxmlDOMDocument::LibxmlDOMDocument(xmlDoc* p_doc)
    : _p_doc(p_doc)
{}

LibxmlDOMDocument::~LibxmlDOMDocument()
{
    if (_p_doc)
        xmlFreeDoc(_p_doc);
}

DOMNode*
LibxmlDOMDocument::getDocumentElement() const
{
    return new LibxmlDOMNode(xmlDocGetRootElement(_p_doc));
}

}
}
