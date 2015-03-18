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

#ifndef WPS_SPI_NMEA_SENTENCE_H_
#define WPS_SPI_NMEA_SENTENCE_H_

#include "nmea/Dataset.h"
#include "Token.h"

#include <list>
#include <string>

namespace WPS {
namespace SPI {
namespace NMEA {

typedef std::pair<int, const Token*> Field;

inline Field
field(int id, const Token* token)
{
    return std::make_pair(id, token);
}

// Encapsulates NMEA sentence format as $[header][payload][checksum]\r\n.
// [payload] is a sequence of comma-separated parameters.
// The format of each is described by Token-derived classes.
// Value of each is stored in a Variant object (in turn Variant's are contained in Dataset).
//
// Hence to define an object to encapsulate particular NMEA sentence,
// you need to instantiate Sentence, passing the header string and a sequence of Token's to constructor.
//
// To provide custom parsing/generation rules, override parse/toString methods.

class Sentence
{
public:

    Sentence(const std::string& head,
             const Field* format,
             size_t fieldCount)
        : _head(head)
        , _format(format, format + fieldCount)
    {}

    virtual ~Sentence()
    {}

    virtual void toString(const Dataset& from, std::string& to) const;
    virtual bool parse(const char* from, size_t length, Dataset& to) const;

    static const char* find(const char* from, size_t& length, const char*& next);

private:

    bool parsePayload(const char* from, size_t length, Dataset& to) const;

private:

    typedef std::list<Field> Format;

    const std::string _head;
    const Format _format;
};

}
}
}

#endif
