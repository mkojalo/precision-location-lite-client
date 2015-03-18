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

#include "spi/XmlHttpRequest.h"
#include "spi/Logger.h"
#include "spi/XmlHttpRequest.h"
#include "spi/StdLibC.h"
#include "spi/Concurrent.h"

#include <map>
#include <set>
#include <list>
#include <memory>

#include <errno.h>
#include <curl/curl.h>

#include "spi/Assert.h"

#define WPS_LOG_CATEGORY "WPS.SPI.CurlXmlHttpRequest"

namespace WPS {
namespace SPI {

class CurlXmlHttpRequest
    : public XmlHttpRequest
{
public:

    CurlXmlHttpRequest()
        : _logger(WPS_LOG_CATEGORY)
        , _statusCode((HttpStatusCode) -1)
        , _curl(NULL)
        , _curlHeaderList(NULL)
    {
        _errorBuffer[0] = '\0';
    }

    ~CurlXmlHttpRequest()
    {
        if (_curlHeaderList != NULL)
            curl_slist_free_all(_curlHeaderList);
    }

    void open(HttpMethod method, const std::string& url)
    {
        _method = method;
        _url = url;
    }

    void setRequestHeader(const std::string& header, const std::string& value)
    {
        _requestHeaders[header] = value;
    }

    ErrorCode send(const std::string& text)
    {
        _requestText = text;

        curl_global_init(CURL_GLOBAL_ALL);

        _curl = curl_easy_init();
        if (! _curl)
        {
            _logger.error("curl_easy_init failed");
            return SPI_ERROR;
        }

        configure();

        CURLcode rc = curl_easy_perform(_curl);

        curl_easy_cleanup(_curl);
        curl_global_cleanup();

        /* we only return HTTP error code when status code wasn't changed;
         * e.g. in case of 407 status code returned by proxy
         * cURL returns CURLE_RECV_ERROR, however we should return HTTP_ERROR_OK
         * in this case
         */
        if (_statusCode == (HttpStatusCode) -1)
            return translateCurlError(rc);
        else
            return SPI_OK;
    }

    std::string getResponseHeader(const std::string& header) const
    {
        Headers::const_iterator it = _responseHeaders.find(header);
        if (it == _responseHeaders.end())
            return "";
        return it->second;
    }

    std::string getResponseData() const
    {
        return _responseText;
    }

    HttpStatusCode getStatusCode() const
    {
        return _statusCode;
    }

    std::string getStatusText() const
    {
        return _statusText;
    }

private:

    void configure()
    {
        curl_easy_setopt(_curl, CURLOPT_URL, _url.c_str());
#ifndef WPS_NO_SSL_CHECK
        curl_easy_setopt(_curl, CURLOPT_SSL_VERIFYPEER, 0);
        curl_easy_setopt(_curl, CURLOPT_SSL_VERIFYHOST, 0);
#endif
        curl_easy_setopt(_curl, CURLOPT_SSLVERSION, CURL_SSLVERSION_SSLv3);

        switch (_method)
        {
        case HTTP_GET:
            curl_easy_setopt(_curl, CURLOPT_HTTPGET, 1);
            break;

        case HTTP_POST:
            curl_easy_setopt(_curl, CURLOPT_POST, 1);
            break;

        case HTTP_HEAD:
            curl_easy_setopt(_curl, CURLOPT_NOBODY, 1);
            break;
        }

        curl_easy_setopt(_curl, CURLOPT_CONNECTTIMEOUT, TIMEOUT);
        curl_easy_setopt(_curl, CURLOPT_TIMEOUT, TIMEOUT);

        if (_method != HTTP_HEAD)
        {
            assert(_curlHeaderList == NULL);

            for (Headers::const_iterator it = _requestHeaders.begin();
                it != _requestHeaders.end();
                ++it)
            {
                std::string header = it->first + ": " + it->second;
                _curlHeaderList = curl_slist_append(_curlHeaderList, header.c_str());
            }

            if (_curlHeaderList)
                curl_easy_setopt(_curl, CURLOPT_HTTPHEADER, _curlHeaderList);
        }

        if (_method == HTTP_POST)
            curl_easy_setopt(_curl, CURLOPT_POSTFIELDS, _requestText.c_str());

        curl_easy_setopt(_curl, CURLOPT_ERRORBUFFER, _errorBuffer);
        curl_easy_setopt(_curl, CURLOPT_WRITEFUNCTION, &writeCallback);
        curl_easy_setopt(_curl, CURLOPT_WRITEDATA, this);
        curl_easy_setopt(_curl, CURLOPT_NOSIGNAL, 1);
        curl_easy_setopt(_curl, CURLOPT_DNS_CACHE_TIMEOUT, 300); // 5 min
#ifndef NDEBUG
        curl_easy_setopt(_curl, CURLOPT_VERBOSE, 1);
        curl_easy_setopt(_curl, CURLOPT_DEBUGFUNCTION, &debugCallback);
        curl_easy_setopt(_curl, CURLOPT_DEBUGDATA, this);
#endif
        curl_easy_setopt(_curl, CURLOPT_HEADERFUNCTION, &headerCallback);
        curl_easy_setopt(_curl, CURLOPT_WRITEHEADER, this);
    }

    static ErrorCode translateCurlError(CURLcode result)
    {
        switch (result)
        {
        case CURLE_OK:
            return SPI_OK;

        case CURLE_UNSUPPORTED_PROTOCOL:
            return SPI_ERROR_PROTOCOL_NOT_SUPPORTED;

        case CURLE_COULDNT_RESOLVE_PROXY:
        case CURLE_COULDNT_RESOLVE_HOST:
            return SPI_ERROR_HOST_UNREACHEABLE;

        case CURLE_OPERATION_TIMEOUTED:
            return SPI_ERROR_TIMED_OUT;

        case CURLE_COULDNT_CONNECT:
        case CURLE_SSL_CONNECT_ERROR:
        case CURLE_RECV_ERROR:
        case CURLE_SEND_ERROR:
            return SPI_ERROR_CONNECTION_REFUSED;

#ifndef NO_CURLE_LOGIN_DENIED
        case CURLE_LOGIN_DENIED:
            return SPI_ERROR_PERMISSION_DENIED;
#endif

        default:
            return SPI_ERROR;
        }
    }

#ifndef NDEBUG
    static int debugCallback(CURL*,
                             curl_infotype info,
                             char* message,
                             size_t length,
                             void* param)
    {
        CurlXmlHttpRequest* _this = reinterpret_cast<CurlXmlHttpRequest*>(param);

        switch (info)
        {
        case CURLINFO_TEXT:
        case CURLINFO_HEADER_IN:
        case CURLINFO_HEADER_OUT:
            break;
        default:
            return 0;
        }

        size_t n = 0;
        char* m = message;
        for (size_t i = 0; i < length; ++i)
        {
            const char c = message[i];
            if (c == '\n' || c == '\r')
            {
                if (n > 0)
                    _this->_logger.debug("%.*s", (int) n, m);

                m = message + i + 1;
                n = 0;
            }
            else
                ++n;
        }

        return 0;
    }
#endif

    static size_t headerCallback(void* ptr,
                                 size_t size,
                                 size_t nmemb,
                                 void* param)
    {
        CurlXmlHttpRequest* _this = reinterpret_cast<CurlXmlHttpRequest*>(param);

        std::string header(reinterpret_cast<char*>(ptr), size * nmemb);

        // parse "header-like" data from cURL
        if (header.length() >= 15 && header.substr(0, 5) == "HTTP/")
        {
            _this->_statusCode = (HttpStatusCode) atoi(header.substr(9, 3).c_str());
            _this->_statusText = header.substr(13, header.length() - 15);
            return size * nmemb;
        }

        // parse actual HTTP headers
        std::string::size_type colon = header.find(": ");
        if (colon != std::string::npos)
        {
            std::string value = header.substr(colon + 2,
                                              header.size() - colon - 4);
            _this->_responseHeaders[header.substr(0, colon)] = value;
            return size * nmemb;
        }

        if (header != "\r\n")
            _this->_logger.warn("unrecognized header: %s", header.c_str());

        return size * nmemb;
    }

    static size_t writeCallback(void* ptr,
                                size_t size,
                                size_t nmemb,
                                void* param)
    {
        CurlXmlHttpRequest* _this = reinterpret_cast<CurlXmlHttpRequest*>(param);

        const size_t len = size * nmemb;

        _this->_responseText.append(reinterpret_cast<const char*>(ptr), len);
        return len;
    }

protected:

    typedef std::map<std::string, std::string> Headers;

    Logger _logger;

    HttpMethod _method;
    std::string _url;

    Headers _requestHeaders;
    Headers _responseHeaders;
    std::string _requestText;
    std::string _responseText;
    HttpStatusCode _statusCode;
    std::string _statusText;
    CURL* _curl;
    curl_slist* _curlHeaderList;
    char _errorBuffer[CURL_ERROR_SIZE];

private:

    static const unsigned int TIMEOUT = 30;
};

/**********************************************************************/
/*                                                                    */
/* XmlHttpRequest::newInstance                                        */
/*                                                                    */
/**********************************************************************/

XmlHttpRequest*
XmlHttpRequest::newInstance()
{
    return new CurlXmlHttpRequest;
}

}
}
