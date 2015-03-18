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

#ifndef WPS_SPI_XML_HTTP_REQUEST_H_
#define WPS_SPI_XML_HTTP_REQUEST_H_

#include <string>
#include <vector>

#include "spi/ErrorCodes.h"

namespace WPS {
namespace SPI {

/**
 * \addtogroup replaceable
 *
 * \b XmlHttpRequest -- a class capable of transferring HTTP, and HTTPS, messages.
 * \li \ref XmlHttpRequest.h
 */
/** @{ */

/**
 * Provides a high-level HTTP/HTTPS client interface for transferring data
 * between a client and a server.
 *
 * @see <a href="http://en.wikipedia.org/wiki/XMLHttpRequest">XMLHttpRequest</a>
 * @see <a href="http://www.w3.org/TR/XMLHttpRequest">The XMLHttpRequest Object</a>
 */
class XmlHttpRequest
{
public:

    /**
     * \ingroup nonreplaceable
     *
     * @see <code>open()</code>
     */
    enum HttpMethod
    {
        HTTP_GET,
        HTTP_POST,
        HTTP_HEAD
    };

     /**
     * \ingroup nonreplaceable
     *
     * HTTP Status Codes
     *
     * @see <a href="http://www.w3.org/Protocols/rfc2616/rfc2616-sec10.html">RFC 2616</a>
     */
    enum HttpStatusCode
    {
        CONTINUE                            = 100,
        SWITCHING_PROTOCOLS                 = 101,

        OK                                  = 200,
        CREATED                             = 201,
        ACCEPTED                            = 202,
        NON_AUTHORITATIVE                   = 203,
        NO_CONTENT                          = 204,
        RESET_CONTENT                       = 205,
        PARTIAL_CONTENT                     = 206,

        MULTIPLE_CHOICES                    = 300,
        MOVED_PERMANENTLY                   = 301,
        FOUND                               = 302,
        SEE_OTHER                           = 303,
        NOT_MODIFIED                        = 304,
        USE_PROXY                           = 305,
        TEMPORARY_REDIRECT                  = 307,

        BAD_REQUEST                         = 400,
        UNAUTHORIZED                        = 401,
        FORBIDDEN                           = 403,
        NOT_FOUND                           = 404,
        METHOD_NOT_ALLOWED                  = 405,
        NOT_ACCEPTABLE                      = 406,
        PROXY_AUTHENTICATION_REQUIRED       = 407,
        REQUEST_TIMEOUT                     = 408,
        CONFLICT                            = 409,
        GONE                                = 410,
        LENGTH_REQUIRED                     = 411,
        PRECONDITION_FAILED                 = 412,
        REQUEST_ENTITY_TOO_LARGE            = 413,
        REQUEST_URI_TOO_LONG                = 414,
        UNSUPPORTED_MEDIA_TYPE              = 415,
        REQUESTED_RANGE_NOT_SATISFIABLE     = 416,
        EXPECTATION_FAILED                  = 417,

        INTERNAL_SERVER_ERROR               = 500,
        NOT_IMPLEMENTED                     = 501,
        BAD_GATEWAY                         = 502,
        SERVICE_UNAVAILABLE                 = 503,
        GATEWAY_TIMEOUT                     = 504,
        HTTP_VERSION_NOT_SUPPORTED          = 505
    };

    /**
     * @return a new instance
     */
    static XmlHttpRequest* newInstance();

    /**
     * Destructor is expected to call abort().
     */
    virtual ~XmlHttpRequest()
    {}

    /**
     * Open a connection to the specified <code>url</code>.
     *
     * @param method the HTTP method to use for this pending request
     * @param url the url to the resource on the server
     */
    virtual void open(HttpMethod method,
                      const std::string& url) =0;

    /**
     * Add (or overwrite) the request header (<code>header</code>) field
     * with <code>value</code>
     *
     * @param header name of the HTTP request header
     * @param value value of the HTTP request header
     *
     * @pre <code>header</code> or <code>value</code> should not contain
     *      line feed or carriage return characters
     */
    virtual void setRequestHeader(const std::string& header,
                                  const std::string& value) =0;

    /**
     * Send data to the server
     *
     * @param data the data to be sent to the server.
     *
     * @note Set the <tt>Content-Size</tt> header
     * @note Follow HTTP redirect
     *
     * @pre A successful call to <code>open()</code> must
     *      precede a call to <code>send()</code>
     */
    virtual ErrorCode send(const std::string& data) =0;

    /**
     * @param header the name of the HTTP header to return
     *
     * @return the value of the HTTP header named <code>header</code> included
     *         in the response.
     */
    virtual std::string getResponseHeader(const std::string& header) const =0;

    /**
     * Retrieve the response text.
     *
     * @return the response text string
     */
    virtual std::string getResponseData() const =0;

    /**
     * @return the status code from the HTTP response line.
     *         \n <i>undefined</i> if the HTTP response hasn't been received yet
     */
    virtual HttpStatusCode getStatusCode() const =0;

    /**
     * @return the status line from the HTTP response line.
     *         \n <i>undefined</i> if the HTTP response hasn't been received yet
     */
    virtual std::string getStatusText() const =0;

protected:

    XmlHttpRequest()
    {}

private:

    /**
     * XmlHttpRequest instances themselves cannot be copied.
     * Implementations may support copying.
     */
    XmlHttpRequest(const XmlHttpRequest&);
    XmlHttpRequest& operator=(const XmlHttpRequest&);
};

/** @} */

}
}

#endif
