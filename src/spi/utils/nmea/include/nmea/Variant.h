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

#ifndef WPS_SPI_NMEA_VARIANT_H_
#define WPS_SPI_NMEA_VARIANT_H_

#include "nmea/Types.h"

#include <string>
#include <algorithm>

#include "spi/Assert.h"

namespace WPS {
namespace SPI {
namespace NMEA {

class Variant
{
public:

    Variant()
        : _type(UNINITIALIZED)
    {}

    ~Variant()
    {
        cleanup();
    }

    Variant(char val)
    {
        _type = CHAR;
        _value.c = val;
    }

    Variant(int val)
    {
        _type = INT;
        _value.d = val;
    }

    Variant(double val)
    {
        _type = DOUBLE;
        _value.f = val;
    }

    Variant(const Satellite& val)
    {
        _type = SATELLITE;
        _value.sat = val;
    }

    Variant(const Time& val)
    {
        _type = TIME;
        _value.time = val;
    }

    Variant(const Date& val)
    {
        _type = DATE;
        _value.date = val;
    }

    Variant(const Variant& other)
    {
        _type = other._type;
        if (_type == STRING)
            fromString(other._value.str);
        else
            _value = other._value;
    }

    Variant(const std::string& val)
    {
        _type = STRING;
        fromString(val);
    }

    operator char() const
    {
        if (_type == UNINITIALIZED)
            return '\0';

        assert(_type == CHAR);
        return _value.c;
    }

    operator int() const
    {
        if (_type == UNINITIALIZED)
            return 0;

        assert(_type == INT);
        return _value.d;
    }

    operator double() const
    {
        if (_type == UNINITIALIZED)
            return 0.0f;

        assert(_type == DOUBLE);
        return _value.f;
    }

    operator const Satellite&() const
    {
        assert(_type == SATELLITE);
        return _value.sat;
    }

    operator const Time&() const
    {
        assert(_type == TIME);
        return _value.time;
    }

    operator const Date&() const
    {
        assert(_type == DATE);
        return _value.date;
    }

    operator std::string() const
    {
        if (_type == UNINITIALIZED)
            return "";

        assert(_type == STRING);
        return _value.str;
    }

    char operator=(char val)
    {
        cleanup();

        _type = CHAR;
        return _value.c = val;
    }

    int operator=(int val)
    {
        cleanup();

        _type = INT;
        return _value.d = val;
    }

    double operator=(double val)
    {
        cleanup();

        _type = DOUBLE;
        return _value.f = val;
    }

    const Satellite& operator=(const Satellite& val)
    {
        cleanup();

        _type = SATELLITE;
        return _value.sat = val;
    }

    const Time& operator=(const Time& val)
    {
        cleanup();

        _type = TIME;
        return _value.time = val;
    }

    const Date& operator=(const Date& val)
    {
        cleanup();

        _type = DATE;
        return _value.date = val;
    }

    const std::string& operator=(const std::string& val)
    {
        cleanup();

        _type = STRING;
        fromString(val);
        return val;
    }

    const Variant& operator=(const Variant& other)
    {
        cleanup();

        _type = other._type;
        if (_type == STRING)
            fromString(other._value.str);
        else
            _value = other._value;

        return other;
    }

private:

    void cleanup()
    {
        if (_type == STRING)
            delete[] _value.str;
    }

    void fromString(const std::string& s)
    {
        _value.str = new char[s.length() + 1];
        s.copy(_value.str, s.length());
        _value.str[s.length()] = '\0';
    }

    enum Type
    {
        UNINITIALIZED,
        CHAR,
        INT,
        DOUBLE,
        SATELLITE,
        TIME,
        DATE,
        STRING
    };

    union Value
    {
        char c;
        int d;
        double f;
        Time time;
        Date date;
        Satellite sat;
        char* str;
    };

    Type _type;
    Value _value;
};

}
}
}

#endif
