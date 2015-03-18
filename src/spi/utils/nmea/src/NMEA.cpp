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

#include "nmea/NMEA.h"

#include "GGA.h"
#include "GSA.h"
#include "GSA1.h"
#include "GSV.h"
#include "RMC.h"
#include "RMC1.h"
#include "GLL.h"

#include <list>

namespace WPS {
namespace SPI {
namespace NMEA {

/**********************************************************************/
/* getSentences                                                       */
/**********************************************************************/

enum
{
    // This is 'wrong' GSA sentence with an additional unknown field
    // emitted by the integrated GPS receiver on the Samsung i780 phone.
    // We keep it private, so the user can't explicitly parse or generate it.
    // We use it internally only when GSA is parsed,
    // putting GSA1 right after GSA in the sentence chain.
    // That way, if 'normal' GSA fails to parse, GSA1 format will be used instead.

    GSA1 = 0x00010000,

    // New version of RMC and VTG that came with NMEA 2.3 standard
    // added an extra field - FIX_MODE_INDICATOR.

    RMC1 = 0x00020000,

    // Incomplete versions of GSV

    GSV1 = 0x00400000,
    GSV2 = 0x00800000,
    GSV3 = 0x01000000
};

typedef std::list<const Sentence*> Sentences;

static const Sentences
getSentences(unsigned int s)
{
    Sentences sentences;

    if (s & GGA)
        sentences.push_back(&sGGA);

    if (s & GSA)
        sentences.push_back(&sGSA);
    if (s & GSA1)
        sentences.push_back(&sGSA1);

    if (s & GSV)
        sentences.push_back(&sGSV);
    if (s & GSV1)
        sentences.push_back(&sGSV1);
    if (s & GSV2)
        sentences.push_back(&sGSV2);
    if (s & GSV3)
        sentences.push_back(&sGSV3);

    if (s & RMC)
        sentences.push_back(&sRMC);
    if (s & RMC1)
        sentences.push_back(&sRMC1);

    if (s & GLL)
        sentences.push_back(&sGLL);

    return sentences;
}

static unsigned int
toSentenceId(const Sentence* pSentence)
{
    if (pSentence == &sGGA)
        return GGA;
    if (pSentence == &sGSA || pSentence == &sGSA1)
        return GSA;
    if (pSentence == &sGSV || pSentence == &sGSV1 || pSentence == &sGSV2 || pSentence == &sGSV3)
        return GSV;
    if (pSentence == &sRMC || pSentence == &sRMC1)
        return RMC;
    if (pSentence == &sGLL)
        return GLL;

    return 0;
}

/**********************************************************************/
/* generate                                                           */
/**********************************************************************/ 

void
generate(const Dataset& from, std::string& to, unsigned int s)
{
    const Sentences sentences = getSentences(s);

    for (Sentences::const_iterator i = sentences.begin();
         i != sentences.end();
         ++i)
    {
        (*i)->toString(from, to);
    }
}

/**********************************************************************/
/* parse                                                              */
/**********************************************************************/ 

size_t
parse(const char* from, Dataset& to)
{
    unsigned int s = ALL;
    return parse(from, to, s);
}

size_t
parse(const char* from, Dataset& to, unsigned int& s)
{
    if (s & GSA)
        s |= GSA1;
    if (s & RMC)
        s |= RMC1;
    if (s & GSV)
        s |= GSV1 | GSV2 | GSV3;

    const Sentences sentences = getSentences(s);

    s = 0;
    size_t length;
    const char* p;
    const char* next = from;
    while ((p = Sentence::find(next, length, next)))
    {
        for (Sentences::const_iterator i = sentences.begin();
             i != sentences.end();
             ++i)
        {
            if ((*i)->parse(p, length, to))
            {
                s |= toSentenceId(*i);
                break;
            }
        }
    }

    return next - from;
}

}
}
}
