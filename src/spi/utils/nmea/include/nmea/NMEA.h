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

#ifndef WPS_SPI_NMEA_H_
#define WPS_SPI_NMEA_H_

#include "nmea/Dataset.h"

#include <string>

namespace WPS {
namespace SPI {
namespace NMEA {

enum
{
    GGA   = 0x00000001,
    GSA   = 0x00000002,
    GSV   = 0x00000004,
    RMC   = 0x00000008,
    GLL   = 0x00000010,

    ALL   = 0x0000FFFF
};

size_t
parse(const char* from, Dataset& to, unsigned int& s);

size_t
parse(const char* from, Dataset& to);

void
generate(const Dataset& from, std::string& to, unsigned int s = ALL);

}
}
}

#endif
