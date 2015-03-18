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

#ifndef WPS_SPI_GLIB_MAIN_LOOP_THREAD_H_
#define WPS_SPI_GLIB_MAIN_LOOP_THREAD_H_

#include "spi/Thread.h"
#include "spi/Concurrent.h"
#include "spi/Logger.h"

#include <pthread.h>

namespace WPS {
namespace SPI {

class GlibMainLoopThread
{
public:

    GlibMainLoopThread()
        : _logger("WPS.SPI.GlibMainLoopThread")
        , _threadReadyEvent(WPS::SPI::Event::newInstance())
        , _thread(0)
    {}

    ErrorCode start()
    {
        if (_thread != 0)
            return SPI_OK;

        _logger.debug("starting");

        int rc = pthread_create(&_thread, NULL, threadProc, this);
        if (rc != 0)
        {
            _logger.error("pthread_create() failed (%d)", rc);
            return SPI_ERROR;
        }

        if (_threadReadyEvent->wait(STARTUP_TIMEOUT) != SPI_OK)
        {
            _logger.error("timed out waiting for thread start");

            pthread_detach(_thread);
            _thread = 0;
            return SPI_ERROR;
        }

        _logger.debug("started");
        return SPI_OK;
    }

    void stop()
    {
        if (_thread != 0)
        {
            _logger.debug("stopping main loop");
            g_main_loop_quit(_loop);

            _logger.debug("waiting for thread");
            pthread_join(_thread, NULL);

            _logger.debug("stopped");
            _thread = 0;
        }
    }

    GMainContext* getContext() const
    {
        assert(_context != NULL);
        return _context;
    }

private:

    static void* threadProc(void* userData)
    {
        ((GlibMainLoopThread*) userData)->threadProc();
        return NULL;
    }

    void threadProc()
    {
        _logger.debug("thread started, initializing context and main loop");

        _context = g_main_context_new();
        _loop = g_main_loop_new(_context, FALSE);

        g_main_context_push_thread_default(_context);

        _logger.debug("main loop initialized");

        _threadReadyEvent->signal();

        _logger.debug("main loop running");

        g_main_loop_run(_loop);

        _logger.debug("main loop stopped, releasing context and main loop");

        g_main_context_pop_thread_default(_context);
        g_main_loop_unref(_loop);
        g_main_context_unref(_context);

        _logger.debug("thread terminated");
    }

private:

    static const unsigned long STARTUP_TIMEOUT = 1000;

    Logger _logger;
    std::auto_ptr<Event> _threadReadyEvent;
    pthread_t _thread;

    GMainContext* _context;
    GMainLoop* _loop;
};

}
}

#endif

