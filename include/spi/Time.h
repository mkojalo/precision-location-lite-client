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

#ifndef WPS_SPI_TIME_H_
#define WPS_SPI_TIME_H_

#ifndef NDEBUG
#  include "spi/StdMath.h"
#endif

#include "spi/StdLibC.h"

#include <string>
#include <cstdlib>

namespace WPS {
namespace SPI {

/**
 * \addtogroup replaceable
 *
 * \b Time -- a portable representation of time
 * \li \ref Time.h
 */

/**
 * \ingroup nonreplaceable
 *
 * A timer with a precision of millisecond.
 *
 * @author Skyhook Wireless
 */
class Timer
{
public:

    /**
     * A timer that has started a long time ago.
     */
    static Timer epoch;

    Timer()
        : _t0(tick())
    {}

    Timer(const Timer& rhs)
        : _t0(rhs._t0)
    {}

    Timer& operator=(const Timer& rhs)
    {
        _t0 = rhs._t0; // No need to check for self-assignment
        return *this;
    }

    /**
     * @return time elapsed (in milliseconds/ticks)
     *         since this instance was created
     */
    unsigned long elapsed() const
    {
        return tick() - _t0;
    }

    unsigned long elapsed(const Timer& now) const
    {
        return now._t0 - _t0;
    }

    long delta(const Timer& rhs) const
    {
        return _t0 - rhs._t0;
    }

    bool operator==(const Timer& rhs) const
    {
        return _t0 == rhs._t0;
    }

    bool operator!=(const Timer& rhs) const
    {
        return _t0 != rhs._t0;
    }

    bool operator<(const Timer& rhs) const
    {
        return delta(rhs) < 0;
    }

    bool operator<=(const Timer& rhs) const
    {
        return delta(rhs) <= 0;
    }

    bool operator>(const Timer& rhs) const
    {
        return delta(rhs) > 0;
    }

    bool operator>=(const Timer& rhs) const
    {
        return delta(rhs) >= 0;
    }

    int compare(const Timer& rhs) const
    {
        if (_t0 == rhs._t0)
            return 0;

        // Note that a newer Timer will be less than an older one
        // (though its _t0 is bigger since it was started later)
        return delta(rhs) > 0 ? -1 : 1;
    }

    void reset()
    {
        _t0 = tick();
    }

    void reset(long initialElapsed)
    {
        _t0 = tick() - initialElapsed;
    }

    void reset(long initialElapsed, const Timer& now)
    {
        _t0 = now._t0 - initialElapsed;
    }

    /**
     * @return <code>this</code> timer as a string.
     */
    std::string toString() const
    {
        char buf[16];
        snprintf(buf, sizeof(buf), "%lu", _t0);
        return buf;
    }

private:

    explicit Timer(unsigned long t0)
        : _t0(t0)
    {}


    /**
     * \ingroup replaceable
     *
     * @return a continuously increasing number of milliseconds
     */
    static unsigned long tick();

private:

    unsigned long _t0;
};

/**
 * \ingroup nonreplaceable
 *
 * Represent a UTC calendar date and time broken down
 * into its components.
 *
 * @see tm in ISO/IEC 9899:1999
 *
 * @author Skyhook Wireless
 */
struct Date
{
    /**
     * Milliseconds after the second - [0, 999].
     */
    unsigned short msec;

    /**
     * Seconds after the minute - [0, 59].
     */
    unsigned short sec;

    /**
     * Minutes after the hour - [0, 59].
     */
    unsigned short min;

    /**
     * Hours since midnight - [0, 23].
     */
    unsigned short hour;

    /**
     * Day of the month - [1, 31].
     */
    unsigned short day;

    /**
     * Months since January - [0, 11].
     */
    unsigned short month;

    /**
     * Years since 1900.
     */
    unsigned short year;
};

/**
 * \ingroup nonreplaceable
 *
 * Fine-grained representation of time
 * with a precision of millisecond.
 *
 * @author Skyhook Wireless
 */
class Time
{
public:

    /**
     * \ingroup replaceable
     *
     * @return a new instance of <code>Time</code>
     *         initialized to the current local time
     */
    static Time now();

    /**
     * \ingroup replaceable
     *
     * @return <code>Time</code> based on <code>date</code>
     *
     * @see mktime in ISO/IEC 9899:1999
     *
     * @note unlike mktime this function takes UTC date/time instead of local
     */
    static Time fromDate(const Date& date);

    /**
     * \ingroup replaceable
     *
     * @return <code>Date</code> based on <code>time</code>
     *
     * @see gmtime in ISO/IEC 9899:1999
     */
    static void toDate(const Time& time, Date& date);

public:

    Time()
        : _msec(0)
    {}

    explicit Time(unsigned long long msec)
        : _msec(msec)
    {}

    Time(unsigned long sec, unsigned int msec)
        : _msec(sec * 1000ULL + msec)
    {}

    Time(const Time& that)
        : _msec(that._msec)
    {}

    explicit Time(const Date& date)
        : _msec(fromDate(date)._msec)
    {}

    Time& operator=(const Time& that)
    {
        _msec = that._msec;
        return *this;
    }

    bool operator==(const Time& that) const
    {
        return _msec == that._msec;
    }

    bool operator!=(const Time& that) const
    {
        return _msec != that._msec;
    }

    bool operator<(const Time& that) const
    {
        return _msec < that._msec;
    }

    bool operator<=(const Time& that) const
    {
        return _msec <= that._msec;
    }

    bool operator>(const Time& that) const
    {
        return _msec > that._msec;
    }

    bool operator>=(const Time& that) const
    {
        return _msec >= that._msec;
    }

    int compare(const Time& that) const
    {
        if (_msec == that._msec)
            return 0;

        return (_msec < that._msec) ? -1 : 1;
    }

    Time& operator+=(long msec)
    {
        _msec += msec;
        return *this;
    }

    Time operator+(long msec) const
    {
        return Time(_msec + msec);
    }

    Time& operator-=(long msec)
    {
        _msec -= msec;
        return *this;
    }

    Time operator-(long msec) const
    {
        return Time(_msec - msec);
    }

    void toDate(Date& date) const
    {
        toDate(*this, date);
    }

    /**
     * @return number of seconds since Epoch (00:00:00 UTC, January 1, 1970).
     *
     * @note POSIX.1 defines seconds since the Epoch as a value to be
     *       interpreted as the number of seconds between a specified time
     *       and the Epoch, according to a formula for conversion from UTC
     *       equivalent to conversion on the naive basis that leap seconds
     *       are ignored and all years divisible by 4 are leap years.
     *       This value is not the same as the actual number of
     *       seconds between the time and the Epoch, because of leap seconds and
     *       because clocks are not required to be synchronized to a standard
     *       reference. The intention is that the interpretation of seconds
     *       since the Epoch values be consistent;
     *       see POSIX.1 Annex B 2.2.2 for further rationale.
     */
    unsigned long sec() const
    {
        return static_cast<unsigned long>(_msec / 1000);
    }

    /**
     * @return the number of milliseconds since <code>sec()</code>
     */
    unsigned int msec() const
    {
        return static_cast<unsigned long>(_msec % 1000);
    }

    /**
     * @return <code>this</code> time as a string.
     */
    std::string toString() const
    {
        char buf[32];
        snprintf(buf, sizeof(buf), "%u%03u", sec(), msec());
        return buf;
    }

private:

    unsigned long long _msec;
};

/** @} */

}
}

#endif
