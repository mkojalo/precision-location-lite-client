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

#define _XOPEN_SOURCE 500

#include <pthread.h>
#include <sys/time.h>
#include <time.h>
#include <errno.h>

#include "spi/Concurrent.h"

namespace WPS {
namespace SPI {

/*********************************************************************/
/* PthreadMutex                                                      */
/*********************************************************************/

class PthreadMutex
    : public Mutex
{
public:

    PthreadMutex()
    {
        pthread_mutexattr_t attrs;
        pthread_mutexattr_init(&attrs);
        pthread_mutexattr_settype(&attrs, PTHREAD_MUTEX_RECURSIVE);
#if !defined(NDEBUG) && defined(PTHREAD_MUTEX_ERRORCHECK)
        pthread_mutexattr_settype(&attrs, PTHREAD_MUTEX_ERRORCHECK);
#endif
        while (pthread_mutex_init(&_mutex, &attrs) == EAGAIN)
            ;
        pthread_mutexattr_destroy(&attrs);
    }

    ~PthreadMutex()
    {
        pthread_mutex_destroy(&_mutex);
    }

    void acquire()
    {
        pthread_mutex_lock(&_mutex);
    }

    void release()
    {
        pthread_mutex_unlock(&_mutex);
    }

private:

    pthread_mutex_t _mutex;
};

Mutex* Mutex::newInstance()
{
    return new PthreadMutex;
}

/*********************************************************************/
/* PthreadEvent                                                      */
/*********************************************************************/

class PthreadEvent
    : public Event
{
public:

    PthreadEvent()
        : _signalled(false)
    {
        pthread_mutex_init(&_mutex, NULL);
        pthread_cond_init(&_cond, NULL);
    }

    ~PthreadEvent()
    {
        pthread_mutex_destroy(&_mutex);
        pthread_cond_destroy(&_cond);
    }

    void signal()
    {
        pthread_mutex_lock(&_mutex);
        _signalled = true;
        pthread_cond_broadcast(&_cond);
        pthread_mutex_unlock(&_mutex);
    }

    void clear()
    {
        pthread_mutex_lock(&_mutex);
        _signalled = false;
        pthread_mutex_unlock(&_mutex);
    }

    int wait(unsigned long milliseconds)
    {
#ifdef __APPLE__
        // Workaround for problem in MacOSX 10.9 beta, pthread_cond_timedwait returned ETIMEDOUT
        if (milliseconds == static_cast<unsigned long>(-1))
            milliseconds = 0xFFFFFFFF;
#endif

        struct timeval now;
        if (gettimeofday(&now, NULL) != 0)
            return -1;

        struct timespec deadline = {0};
        deadline.tv_sec = now.tv_sec + milliseconds / 1000;
        deadline.tv_nsec = now.tv_usec * 1000 + (milliseconds % 1000) * 1000000;

        if (deadline.tv_nsec > 1000000000)
        {
            deadline.tv_sec += deadline.tv_nsec / 1000000000;
            deadline.tv_nsec = deadline.tv_nsec % 1000000000;
        }

        pthread_mutex_lock(&_mutex);

        int rc = 0;
        while (! _signalled && rc == 0)
        {
            int status = pthread_cond_timedwait(&_cond, &_mutex, &deadline);

            if (status == ETIMEDOUT)
                rc = 1;
            else if (status != 0) // should not happen
                rc = -1;
        }

        pthread_mutex_unlock(&_mutex);
        return rc;
    }

private:

    pthread_mutex_t _mutex;
    pthread_cond_t  _cond;
    bool _signalled;
};

Event* Event::newInstance()
{
    return new PthreadEvent;
}

}
}
