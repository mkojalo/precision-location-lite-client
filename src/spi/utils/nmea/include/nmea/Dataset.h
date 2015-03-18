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

#ifndef WPS_SPI_NMEA_DATASET_H_
#define WPS_SPI_NMEA_DATASET_H_

#include "Fields.h"
#include "Variant.h"

#include <map>

namespace WPS {
namespace SPI {
namespace NMEA {

class Dataset
{
public:

    void reset()
    {
        _data.clear();
    }

    bool isPresent(int field) const
    {
        return _data.find(field) != _data.end();
    }

    Variant get(int field) const
    {
        Container::const_iterator it = _data.find(field);
        if (it == _data.end())
            return Variant();

        return it->second;
    }

    void set(int field, const Variant& value)
    {
        std::pair<Container::iterator, bool> r =
            _data.insert(Container::value_type(field, value));

        if (!r.second)
            r.first->second = value;
    }

    void remove(int field)
    {
        _data.erase(field);
    }

    void copy(int field, Dataset& to) const
    {
        if (isPresent(field))
            to.set(field, get(field));
        else
            to.remove(field);
    }

private:

    typedef std::map<int, Variant> Container;

    Container _data;
};

}
}
}

#endif
