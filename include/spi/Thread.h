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

#ifndef WPS_SPI_THREAD_H_
#define WPS_SPI_THREAD_H_

#include "spi/Time.h"

namespace WPS {
namespace SPI {

/**
 * \addtogroup replaceable
 *
 * \b Thread -- access to threads, particularly to suspend them
 * \li \ref Thread.h
 */
/** @{ */

/**
 * Encapsulate access to threads
 *
 * @author Skyhook Wireless
 */
struct Thread
{
    /**
     * Suspend the execution of the current thread
     * for the specified interval.
     *
     * @param milliseconds the interval, in milliseconds,
     *        to suspend execution for.
     *
     * @deprecated Instead use \c Event::wait().
     */
    static void msleep(unsigned long milliseconds);

    /**
     * Get the calling thread's ID.
     */
    static unsigned long id();
};

/** @} */

}
}

#endif
