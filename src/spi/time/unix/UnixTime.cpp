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

#include "spi/Time.h"

#ifdef __APPLE__
#  include <mach/mach.h>
#  include <mach/mach_time.h>
#endif
#include <sys/time.h>
#include <time.h>
#ifdef HAVE_CLOCK_GETTIME
#  include <unistd.h>
#endif

namespace WPS {
namespace SPI {

Timer
Timer::epoch(0);

unsigned long
Timer::tick()
{
#ifdef __APPLE__

    // see http://developer.apple.com/mac/library/qa/qa2004/qa1398.html

    static mach_timebase_info_data_t sTimebaseInfo;
    if (sTimebaseInfo.denom == 0)
        (void) mach_timebase_info(&sTimebaseInfo);

    uint64_t now = mach_absolute_time();
    return (now / 1000000) * sTimebaseInfo.numer / sTimebaseInfo.denom;

#else // !__APPLE__

#  ifdef HAVE_CLOCK_GETTIME
    struct timespec ts;
    if (::clock_gettime(CLOCK_MONOTONIC, &ts) == 0)
        return ts.tv_sec * 1000 + ts.tv_nsec / 1000000;
#  endif

    struct timeval tv;
    if (::gettimeofday(&tv, NULL) == 0)
        return tv.tv_sec * 1000 + tv.tv_usec / 1000;

    return 0;

#endif // !__APPLE__
}

Time
Time::now()
{
    struct timeval tv;
    if (::gettimeofday(&tv, NULL) == -1)
        return Time();
    return Time(tv.tv_sec, tv.tv_usec / 1000);
}

Time
Time::fromDate(const Date& date)
{
    tm t = { date.sec,
             date.min,
             date.hour,
             date.day,
             date.month,
             date.year,
             0, 0, 0 };

    return Time(static_cast<unsigned long>(::timegm(&t)),
                date.msec);
}

void
Time::toDate(const Time& time, Date& date)
{
    const time_t s = time.sec();
    tm* t = ::gmtime(&s);

    date.msec = time.msec();
    date.sec = t->tm_sec;
    date.min = t->tm_min;
    date.hour = t->tm_hour;
    date.day = t->tm_mday;
    date.month = t->tm_mon;
    date.year = t->tm_year;
}

}
}
