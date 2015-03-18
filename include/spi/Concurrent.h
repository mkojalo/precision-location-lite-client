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

#ifndef WPS_SPI_CONCURRENT_H_
#define WPS_SPI_CONCURRENT_H_

#include "spi/Assert.h"

namespace WPS {
namespace SPI {

/**
 * \addtogroup replaceable
 *
 * \b Concurrent -- synchronization interfaces
 * \li \ref Concurrent.h
 */
/** @{ */

/**
 * A mutual exclusion object.
 *
 * @author Skyhook Wireless
 */
class Mutex
{
public:

    static Mutex* newInstance();

    virtual void acquire() =0;

    virtual void release() =0;

    virtual ~Mutex()
    {}

protected:

    Mutex()
    {}

private:

    /**
     * Mutex instances themselves cannot be copied.
     * Implementations may support copying.
     */
    Mutex(const Mutex&);
    Mutex& operator=(const Mutex&);
};

/**
 * A event object.
 *
 * @author Skyhook Wireless
 */
class Event
{
public:

    /**
     * Create a new \c Event object.
     * Initially the \c Event is not signaled.
     */
    static Event* newInstance();

    /**
     * \ingroup nonreplaceable
     *
     * Create a new \c Event object.
     * Initially the \c Event is signaled.
     *
     * @see <code>newInstance()</code>
     */
    static Event* newSignaledInstance()
    {
        Event* event = newInstance();
        event->signal();
        return event;
    }

    /**
     * Signal this \c Event object.
     */
    virtual void signal() =0;

    /**
     * Clear this \c Event object.
     */
    virtual void clear() =0;

    /**
     * Wait until this \c Event is signaled.
     *
     * \note This method doesn't \c clear() the object.
     *
     * \see \c signal().
     *
     * @return <code>0</code> if this <code>Event</code> is signaled
     *         <code>> 0</code> if the timeout expired
     *         <code>< 0</code> if an error occurred
     */
    virtual int wait(unsigned long milliseconds) =0;

    virtual ~Event()
    {}

protected:

    Event()
    {}

private:

    /**
     * Event instances themselves cannot be copied.
     * Implementations may support copying.
     */
    Event(const Event&);
    Event& operator=(const Event&);
};

/** @} */

/**
 * \ingroup nonreplaceable
 */
class Guard
{
public:

    Guard(Mutex* mutex)
        : _mutex(mutex)
    {
        assert(mutex);
        _mutex->acquire();
    }

    ~Guard()
    {
        _mutex->release();
    }

private:

    Mutex* _mutex;
};

}
}

#endif
