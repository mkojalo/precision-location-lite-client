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

#ifndef WPS_SPI_NMEA_GSV_H_
#define WPS_SPI_NMEA_GSV_H_

#include "Sentence.h"
#include "Tokens.h"

namespace WPS {
namespace SPI {
namespace NMEA {

// There may be up to 3 GSV's in a raw describing up to 12 satellites:
//
// GSV(1/3) 1  2  3  4
// GSV(2/3) 5  6  7  8
// GSV(3/3) 9 10 11 12
// 
// So we need to override base generation and parsing methods
// to map to the sequence of 12 satellites in Dataset (SATELLITE_01 - SATELLITE_12).
//
// NOTE: There are 16-channel GPS units than emit 4 GSV sentences
//       carrying up to 16 satellites.
//       So far we know 2 devices: integrated GPS on SE Xperia X1 and
//       UBlox GPS mentined on http://www.gpsinformation.org/dale/nmea.htm.
//
//       To support those devices, we added 4 more satellites
//       to Dataset: SATELLITE_13 - SATELLITE_16.

enum
{
    GSV_MESSAGE_COUNT = 100,
    GSV_MESSAGE_NUMBER,
    GSV_SATELLITE_01_PRN,
    GSV_SATELLITE_01_ELV,
    GSV_SATELLITE_01_AZM,
    GSV_SATELLITE_01_SNR,
    GSV_SATELLITE_02_PRN,
    GSV_SATELLITE_02_ELV,
    GSV_SATELLITE_02_AZM,
    GSV_SATELLITE_02_SNR,
    GSV_SATELLITE_03_PRN,
    GSV_SATELLITE_03_ELV,
    GSV_SATELLITE_03_AZM,
    GSV_SATELLITE_03_SNR,
    GSV_SATELLITE_04_PRN,
    GSV_SATELLITE_04_ELV,
    GSV_SATELLITE_04_AZM,
    GSV_SATELLITE_04_SNR
};

#define GSV_FORMAT_HEADER field(GSV_MESSAGE_COUNT,  &tInt1), \
                          field(GSV_MESSAGE_NUMBER, &tInt1), \
                          field(SAT_IN_VIEW,        &tInt2)  \

#define GSV_FORMAT_SAT(n) field(GSV_SATELLITE_##n##_PRN, &tInt2), \
                          field(GSV_SATELLITE_##n##_ELV, &tInt2), \
                          field(GSV_SATELLITE_##n##_AZM, &tInt3), \
                          field(GSV_SATELLITE_##n##_SNR, &tInt2)  \

const Field fGSV1[] = { GSV_FORMAT_HEADER,
                        GSV_FORMAT_SAT(01)
                      };

const Field fGSV2[] = { GSV_FORMAT_HEADER,
                        GSV_FORMAT_SAT(01),
                        GSV_FORMAT_SAT(02)
                      };

const Field fGSV3[] = { GSV_FORMAT_HEADER,
                        GSV_FORMAT_SAT(01),
                        GSV_FORMAT_SAT(02),
                        GSV_FORMAT_SAT(03)
                      };

const Field fGSV[] =  { GSV_FORMAT_HEADER,
                        GSV_FORMAT_SAT(01),
                        GSV_FORMAT_SAT(02),
                        GSV_FORMAT_SAT(03),
                        GSV_FORMAT_SAT(04)
                      };

class GSVSentence
    : public Sentence
{
public:

    GSVSentence(const Field* format, size_t fieldCount)
        : Sentence("GPGSV", format, fieldCount)
    {}

    virtual void toString(const Dataset& from, std::string& to) const;
    virtual bool parse(const char* from, size_t length, Dataset& to) const;
};

const GSVSentence sGSV1(fGSV1, sizeof(fGSV1) / sizeof(fGSV1[0]));
const GSVSentence sGSV2(fGSV2, sizeof(fGSV2) / sizeof(fGSV2[0]));
const GSVSentence sGSV3(fGSV3, sizeof(fGSV3) / sizeof(fGSV3[0]));
const GSVSentence sGSV (fGSV,  sizeof(fGSV)  / sizeof(fGSV[0]));

}
}
}

#endif
