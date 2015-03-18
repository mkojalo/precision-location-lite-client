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

#ifndef WPS_SPI_DOM_H_
#define WPS_SPI_DOM_H_

#include <string>
#include <vector>

namespace WPS {
namespace SPI {

class DOMNodeList;

/**
 * \addtogroup replaceable
 *
 * \b XML -- XML processing
 * \li \ref DOM.h -- simplified, read-only, DOM interfaces
 * \li \ref XmlParser.h -- <a href="http://www.saxproject.org">XmlParser</a> interface
 *                         for parsing XML documents
 */
/** @{ */

/**
 * @see <a href="http://www.w3.org/TR/2000/REC-DOM-Level-2-Core-20001113/core.html#ID-1950641247">Document Object Model (DOM) Level 2 Core Specification</a>
 *
 * @author Skyhook Wireless
 */
class DOMNode
{
public:

    /**
     * For an <tt>ELEMENT_NODE</tt>, return the element name.
     * \n
     * For an <tt>ATTRIBUTE_NODE</tt>, return the attribute name.
     *
     * @return the node name.
     */
    virtual std::string getNodeName() const =0;

    /**
     * For an <tt>ELEMENT_NODE</tt>, return the text directly contained
     * by this element.
     * <br>
     * For an <tt>ATTRIBUTE_NODE</tt>, return the attribute value.
     *
     * @return the node value.
     */
    virtual std::string getNodeValue() const =0;

    /**
     * @return the namespace URI for this node.
     */
    virtual std::string getNamespaceURI() const =0;

    /**
     * @return the namespace prefix for this node.
     */
    virtual std::string getPrefix() const =0;

    /**
     * @return the local name of this node.
     *
     * @see getNodeName
     */
    virtual std::string getLocalName() const =0;

    /**
     * @note Unlike standard DOM, the caller is responsible for releasing
     *       the pointer returned.
     *
     * @return all the children of this node.
     *         If this node has no child, return an empty list
     *         (not <tt>NULL</tt>).
     */
    virtual DOMNodeList* getChildNodes() const =0;

    /**
     * Return the value of the attribute named \a localName
     * in namespace \a namespaceURI.
     * <br>
     * This method only makes sense on <tt>ELEMENT_NODE</tt>'s.
     * For <tt>ATTRIBUTE_NODE</tt> the return value is undefined
     * (although typically an empty string).
     * \par
     * Functionally equivalent to:
     * \code
     *   getAttributeNodeNS(namespaceURI, localName)->getNodeValue();
     * \endcode
     *
     * @see getAttributeNodeNS
     *
     * @param namespaceURI the namespace of the attribute's name
     * @param localName the local name of the attribute's name
     * @return the attribute's value.
     */
    virtual std::string getAttributeNS(const std::string& namespaceURI,
                                       const std::string& localName) const =0;

    /**
     * Return the attribute node named \a localName
     * in namespace \a namespaceURI.
     * <br>
     * This method only makes sense on <tt>ELEMENT_NODE</tt>'s.
     * For <tt>ATTRIBUTE_NODE</tt> the return value is <tt>NULL</tt>.
     *
     * @note Unlike standard DOM, the caller is responsible for releasing
     *       the pointer returned.
     *
     * @param namespaceURI the namespace of the attribute's name
     * @param localName the local name of the attribute's name
     * @return the attribute node.
     */
    virtual DOMNode* getAttributeNodeNS(const std::string& namespaceURI,
                                        const std::string& localName) const =0;

    virtual ~DOMNode()
    {}

protected:

    DOMNode()
    {}

private:

    /**
     * DOMNode instances themselves cannot be copied.
     * Implementations may support copying.
     */
    DOMNode(const DOMNode&);
    DOMNode& operator=(const DOMNode&);
};

/**
 * @see <a href="http://www.w3.org/TR/2000/REC-DOM-Level-2-Core-20001113/core.html#ID-536297177">Document Object Model (DOM) Level 2 Core Specification</a>
 *
 * @author Skyhook Wireless
 */
class DOMNodeList
{
public:

    /**
     * @note Unlike standard DOM, the caller is responsible for releasing
     *       the pointer returned.
     *
     * @return the node at index \c index.
     *         The behavior is undefined if \c index is outside
     *         the range [<tt>0</tt>, <tt>getLength() - 1</tt>].
     */
    virtual DOMNode* getItem(unsigned long index) const =0;

    /**
     * @return the number of elements in this list.
     */
    virtual unsigned long getLength() const =0;

    virtual ~DOMNodeList()
    {}

protected:

    DOMNodeList()
    {}

private:

    /**
     * DOMNodeList instances themselves cannot be copied.
     * Implementations may support copying.
     */
    DOMNodeList(const DOMNodeList&);
    DOMNodeList& operator=(const DOMNodeList&);
};

/**
 * @see <a href="http://www.w3.org/TR/2000/REC-DOM-Level-2-Core-20001113/core.html#i-Document">Document Object Model (DOM) Level 2 Core Specification</a>
 *
 * @author Skyhook Wireless
 */
class DOMDocument
{
public:

    /**
     * @note Unlike standard DOM, the caller is responsible for releasing
     *       the pointer returned.
     *
     * @return the root element of this document.
     */
    virtual DOMNode* getDocumentElement() const =0;

    virtual ~DOMDocument()
    {}

protected:

    DOMDocument()
    {}

private:

    /**
     * DOMDocument instances themselves cannot be copied.
     * Implementations may support copying.
     */
    DOMDocument(const DOMDocument&);
    DOMDocument& operator=(const DOMDocument&);
};

/** @} */

}
}

#endif
