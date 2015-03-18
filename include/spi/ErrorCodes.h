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

#ifndef WPS_SPI_ERROR_CODES_H_
#define WPS_SPI_ERROR_CODES_H_

namespace WPS {
namespace SPI {

/**
 * \ingroup nonreplaceable
 *
 * WPS Error Codes
 *
 * @since WPS API 3.0
 *
 * @author Skyhook Wireless
 */
enum ErrorCode
{
    SPI_OK = 0,
    SPI_ERROR_PERMISSION_DENIED,
    SPI_ERROR_CONNECTION_REFUSED,
    SPI_ERROR_CONNECTION_RESET,
    SPI_ERROR_PROTOCOL_NOT_SUPPORTED,
    SPI_ERROR_HOST_UNREACHEABLE,
    SPI_ERROR_TIMED_OUT,
    SPI_ERROR_IO,
    SPI_ERROR_NOT_READY,
    SPI_ERROR_NO_MEMORY,
    SPI_ERROR = -1
};

}
}

#endif
