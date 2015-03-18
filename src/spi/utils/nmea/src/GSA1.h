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

#ifndef WPS_SPI_NMEA_GSA1_H_
#define WPS_SPI_NMEA_GSA1_H_

#include "Sentence.h"
#include "Tokens.h"

namespace WPS {
namespace SPI {
namespace NMEA {

enum
{
    GSA_UNKNOWN_1 = 1000 // Unknown field in GSA sentence emitted on Samsung i780
};

const Field fGSA1[] = { field(FIX_MODE,      &tChar),
                        field(FIX_TYPE,      &tInt1),
                        field(SAT_IN_USE_01, &tInt2),
                        field(SAT_IN_USE_02, &tInt2),
                        field(SAT_IN_USE_03, &tInt2),
                        field(SAT_IN_USE_04, &tInt2),
                        field(SAT_IN_USE_05, &tInt2),
                        field(SAT_IN_USE_06, &tInt2),
                        field(SAT_IN_USE_07, &tInt2),
                        field(SAT_IN_USE_08, &tInt2),
                        field(SAT_IN_USE_09, &tInt2),
                        field(SAT_IN_USE_10, &tInt2),
                        field(SAT_IN_USE_11, &tInt2),
                        field(SAT_IN_USE_12, &tInt2),
                        field(GSA_UNKNOWN_1, &tInt2),
                        field(PDOP,          &tFloat3_1),
                        field(HDOP,          &tFloat3_1),
                        field(VDOP,          &tFloat3_1)
                      };

const Sentence sGSA1("GPGSA", fGSA1, sizeof(fGSA1) / sizeof(fGSA1[0]));

}
}
}

#endif
