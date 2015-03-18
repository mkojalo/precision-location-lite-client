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
 
#include <errno.h>

#include "spi/ErrorCodes.h"

namespace WPS {
namespace SPI {

inline
ErrorCode
toErrorCode(int errNo)
{
    switch (errNo)
    {
    case 0:
        return SPI_OK;

    case EPERM:
    case EACCES:
        return SPI_ERROR_PERMISSION_DENIED;

    case ECONNREFUSED:
        return SPI_ERROR_CONNECTION_REFUSED;

    case ECONNRESET:
        return SPI_ERROR_CONNECTION_RESET;

    case EPROTONOSUPPORT:
        return SPI_ERROR_PROTOCOL_NOT_SUPPORTED;

    case EHOSTUNREACH:
    case EHOSTDOWN:
        return SPI_ERROR_HOST_UNREACHEABLE;

    case ETIMEDOUT:
        return SPI_ERROR_TIMED_OUT;

    case EIO:
        return SPI_ERROR_IO;
    };

    return SPI_ERROR;
}

}
}
