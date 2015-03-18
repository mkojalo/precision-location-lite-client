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

#include "spi/Thread.h"

#include <unistd.h>
#include <pthread.h>

namespace WPS {
namespace SPI {

void
Thread::msleep(unsigned long milliseconds)
{
    ::usleep(milliseconds*1000);
}

unsigned long
Thread::id()
{
#if defined __APPLE__ || defined __CYGWIN__
    return reinterpret_cast<unsigned long>(pthread_self());
#else
    return pthread_self();
#endif
}

}
}
