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

#ifndef WPS_SPI_NMEA_TOKEN_H_
#define WPS_SPI_NMEA_TOKEN_H_

#include "nmea/Variant.h"
#include "spi/StdLibC.h"

#include <string>

#include "spi/Assert.h"

namespace WPS {
namespace SPI {
namespace NMEA {

/**********************************************************************/
/* Token                                                              */
/**********************************************************************/ 

class Token
{
public:

    Token()
    {}

    virtual ~Token()
    {}

    virtual void toString(const Variant& from,
                          std::string& to) const = 0;

    virtual bool parse(const char* from,
                       size_t length,
                       Variant& to) const = 0;
};

/**********************************************************************/
/* IntToken                                                           */
/**********************************************************************/ 

template <int digits>
class IntToken
    : public Token
{
public:

    IntToken()
    {}

    void toString(const Variant& from, std::string& to) const
    {
        char format[6];
        snprintf(format, sizeof format, "%%0%dd", digits);

        char buffer[digits + 2]; // additional room for minus sign and \0
        snprintf(buffer, sizeof buffer, format, from.operator int());

        to += buffer;
    }

    bool parse(const char* from, size_t length, Variant& to) const
    {
        to = atoi(std::string(from, length)); // FIXME: error handling
        return true;
    }
};

/**********************************************************************/
/* FloatToken                                                         */
/**********************************************************************/ 

template <int totalDigits, int decimalDigits>
class FloatToken
    : public Token
{
public:

    FloatToken()
    {}

    void toString(const Variant& from, std::string& to) const
    {
        char format[9];
        snprintf(format, sizeof format, "%%0%d.%df", totalDigits, decimalDigits);

        char buffer[totalDigits + 3]; // additional room for dot, minus sign and \0
        snprintf(buffer, sizeof buffer, format, from.operator double());

        to += buffer;
    }

    bool parse(const char* from, size_t length, Variant& to) const
    {
        // NOTE: since we don't handle parsing errors here,
        //       we'll check for "nan" explicitly
        const std::string s(from, length);
        if (s == "nan")
            return false;

        to = atof(s); // FIXME: error handling
        return true;
    }
};

/**********************************************************************/
/* CharToken                                                          */
/**********************************************************************/ 

class CharToken
    : public Token
{
public:

    CharToken()
    {}

    void toString(const Variant& from, std::string& to) const
    {
        to += from.operator char();
    }

    bool parse(const char* from, size_t length, Variant& to) const
    {
        if (length != 1)
            return false;

        to = from[0];
        return true;
    }
};

/**********************************************************************/
/* StringToken                                                        */
/**********************************************************************/ 

class StringToken
    : public Token
{
public:

    StringToken()
    {}

    void toString(const Variant& from, std::string& to) const
    {
        to += from.operator std::string();
    }

    bool parse(const char* from, size_t length, Variant& to) const
    {
        to = std::string(from, length);
        return true;
    }
};

/**********************************************************************/
/* TimeToken                                                          */
/**********************************************************************/ 

class TimeToken
    : public Token
{
public:

    TimeToken()
    {}

    void toString(const Variant& from, std::string& to) const
    {
        const Time& time = from;

        assert(time.hour >= 0 && time.hour < 24);
        assert(time.minute >= 0 && time.minute < 60);
        assert(time.second >= 0 && time.second <= 60);
        assert(time.hsecond < 1000);

        int hsecond = time.hsecond;
        if (hsecond > 0 && hsecond < 10)
            hsecond *= 10; // tenth to hundredth
        else if (hsecond >= 100)
            hsecond /= 10; // thousands to hundredth

        char buffer[sizeof("hhmmss.ss")];
        snprintf(buffer, sizeof buffer, "%02d%02d%02d.%02d", time.hour,
                                                             time.minute,
                                                             time.second,
                                                             hsecond);
        to += buffer;
    }

    bool parse(const char* from, size_t length, Variant& to) const
    {
        if (length < sizeof("hhmmss")-1 || length > sizeof("hhmmss.sss")-1)
            return false;

        Time time;
        time.hour = atoi(std::string(from, 2));
        time.minute = atoi(std::string(from + 2, 2));
        time.second = atoi(std::string(from + 4, 2));

        if (length > sizeof("hhmmss")-1)
        {
            const size_t hsec_length = length - (sizeof("hhmmss.")-1);
            const int hsec = atoi(std::string(from + 7, hsec_length));

            switch (hsec_length)
            {
            case 1:
                time.hsecond = hsec * 10; // tenth to hundredth
                break;

            case 2:
                time.hsecond = hsec;
                break;

            case 3:
                time.hsecond = hsec / 10; // thousandth to hundredth
                break;

            default:
                assert(false);
            }
        }

        to = time;
        return true;
    }
};

/**********************************************************************/
/* DateToken                                                          */
/**********************************************************************/ 

class DateToken
    : public Token
{
public:

    DateToken()
    {}

    void toString(const Variant& from, std::string& to) const
    {
        const Date& date = from;

        assert(date.day >= 1 && date.day <= 31);
        assert(date.month >= 1 && date.month <= 12);
        assert(date.year >= 0 && date.year <= 99);

        char buffer[sizeof("ddmmyy")];
        snprintf(buffer, sizeof buffer, "%02d%02d%02d", date.day,
                                                        date.month,
                                                        date.year);
        to += buffer;
    }

    bool parse(const char* from, size_t length, Variant& to) const
    {
        if (length != sizeof("ddmmyy")-1)
            return false;

        // FIXME: error handling
        Date date;
        date.day = atoi(std::string(from, 2));
        date.month = atoi(std::string(from + 2, 2));
        date.year = atoi(std::string(from + 4, 2));

        to = date;
        return true;
    }
};

}
}
}

#endif
