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

#include "GSV.h"

namespace WPS {
namespace SPI {
namespace NMEA {

static const int SAT_IN_MESSAGE = 4;
static const int MAX_GSV_MESSAGES = MAX_SAT_IN_VIEW / SAT_IN_MESSAGE;

void
GSVSentence::toString(const Dataset& from, std::string& to) const
{
    int satCount = from.get(SAT_IN_VIEW);
    if (satCount <= 0) // protect against corrupted data
        return;

    // NOTE: No support for more than 16 satellites in view (4 GSV messages)
    if (satCount > MAX_SAT_IN_VIEW)
        satCount = MAX_SAT_IN_VIEW;

    int msgCount = satCount / SAT_IN_MESSAGE + (satCount % SAT_IN_MESSAGE ? 1 : 0);

    for (int msgNo = 1; msgNo <= msgCount; ++msgNo)
    {
        Dataset gsv;
        from.copy(SAT_IN_VIEW, gsv);

        gsv.set(GSV_MESSAGE_COUNT, msgCount);
        gsv.set(GSV_MESSAGE_NUMBER, msgNo);

        for (int satNo = 0; satNo < SAT_IN_MESSAGE; ++satNo)
        {
            int i = SATELLITE_01 + (msgNo - 1) * SAT_IN_MESSAGE + satNo;
            if (from.isPresent(i))
            {
                const Satellite sat = from.get(i);

                gsv.set(GSV_SATELLITE_01_PRN + satNo * 4, sat.prn);
                gsv.set(GSV_SATELLITE_01_ELV + satNo * 4, sat.elevation);
                gsv.set(GSV_SATELLITE_01_AZM + satNo * 4, sat.azimuth);
                gsv.set(GSV_SATELLITE_01_SNR + satNo * 4, sat.snr);
            }
        }

        Sentence::toString(gsv, to);
    }
}

bool
GSVSentence::parse(const char* from, size_t length, Dataset& to) const
{
    Dataset gsv;
    if (!Sentence::parse(from, length, gsv))
        return false;

    int msgCount = gsv.get(GSV_MESSAGE_COUNT);
    if (msgCount <= 0 || msgCount > MAX_GSV_MESSAGES)
        return false;

    int msgNo = gsv.get(GSV_MESSAGE_NUMBER);
    if (msgNo <= 0 || msgNo > msgCount)
        return false;

    if (msgNo == 1 && msgCount < MAX_GSV_MESSAGES)
    {
        for (int i = msgCount * SAT_IN_MESSAGE;
             i < MAX_GSV_MESSAGES * SAT_IN_MESSAGE;
             ++i)
        {
            to.remove(SATELLITE_01 + i);
        }
    }

    gsv.copy(SAT_IN_VIEW, to);

    for (int satNo = 0; satNo < SAT_IN_MESSAGE; ++satNo)
    {
        const int i = SATELLITE_01 + (msgNo - 1) * SAT_IN_MESSAGE + satNo;

        // make sure all fields for the satellite are present
        if (gsv.isPresent(GSV_SATELLITE_01_PRN + satNo * 4)
            && gsv.isPresent(GSV_SATELLITE_01_ELV + satNo * 4)
            && gsv.isPresent(GSV_SATELLITE_01_AZM + satNo * 4)
            && gsv.isPresent(GSV_SATELLITE_01_SNR + satNo * 4))
        {
            Satellite sat;
            sat.prn       = gsv.get(GSV_SATELLITE_01_PRN + satNo * 4);
            sat.elevation = gsv.get(GSV_SATELLITE_01_ELV + satNo * 4);
            sat.azimuth   = gsv.get(GSV_SATELLITE_01_AZM + satNo * 4);
            sat.snr       = gsv.get(GSV_SATELLITE_01_SNR + satNo * 4);

            to.set(i, sat);
        }
        else
            to.remove(i);
    }

    return true;
}

}
}
}
