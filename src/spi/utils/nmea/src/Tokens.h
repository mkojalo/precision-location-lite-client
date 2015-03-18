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

#ifndef WPS_SPI_NMEA_TOKENS_H_
#define WPS_SPI_NMEA_TOKENS_H_

#include "Token.h"

namespace WPS {
namespace SPI {
namespace NMEA {

// Instantiate all tokens needed for NMEA sentences

const CharToken        tChar;
const StringToken      tString;
const TimeToken        tTime;
const DateToken        tDate;
const IntToken<1>      tInt1;
const IntToken<2>      tInt2;
const IntToken<3>      tInt3;
const IntToken<4>      tInt4;
const FloatToken< 3,1> tFloat3_1;
const FloatToken< 9,4> tFloat9_4;
const FloatToken<10,4> tFloat10_4;

}
}
}

#endif
