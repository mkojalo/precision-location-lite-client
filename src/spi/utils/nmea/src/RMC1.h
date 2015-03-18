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

#ifndef WPS_SPI_NMEA_RMC1_H_
#define WPS_SPI_NMEA_RMC1_H_

#include "Sentence.h"
#include "Tokens.h"

namespace WPS {
namespace SPI {
namespace NMEA {

const Field fRMC1[] = { field(TIME,               &tTime),
                        field(STATUS,             &tChar),
                        field(LATITUDE,           &tFloat9_4),
                        field(LATITUDE_NS,        &tChar),
                        field(LONGITUDE,          &tFloat10_4),
                        field(LONGITUDE_EW,       &tChar),
                        field(SPEED,              &tFloat3_1),
                        field(DIRECTION,          &tFloat3_1),
                        field(DATE,               &tDate),
                        field(DECLINATION,        &tFloat3_1),
                        field(DECLINATION_EW,     &tChar),
                        field(FIX_MODE_INDICATOR, &tChar)
                      };

const Sentence sRMC1("GPRMC", fRMC1, sizeof(fRMC1) / sizeof(fRMC1[0]));

}
}
}

#endif
