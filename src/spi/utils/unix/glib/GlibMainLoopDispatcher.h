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

#ifndef WPS_SPI_GLIB_MAIN_LOOP_GlibMainLoopDispatcher_H_
#define WPS_SPI_GLIB_MAIN_LOOP_GlibMainLoopDispatcher_H_

#include "spi/Logger.h"
#include "spi/Concurrent.h"
#include "spi/ErrorCodes.h"

#include <glib.h>
#include <memory>

namespace WPS {
namespace SPI {

static gboolean
prepare(GSource* source, gint* timeout)
{
    *timeout = -1;
    return TRUE;
}

static gboolean
check(GSource* source)
{
    return TRUE;
}

static gboolean
dispatch(GSource* source,
         GSourceFunc sourceFunc,
         gpointer user_data)
{
    return sourceFunc(user_data);
}

static
GSourceFuncs sourceFuncs = { prepare, check, dispatch, NULL };

class GlibMainLoopDispatcher
{
public:

    typedef ErrorCode (*Callback)(void*);

    GlibMainLoopDispatcher()
        : _logger("WPS.SPI.GlibMainLoopDispatcher")
        , _mutex(Mutex::newInstance())
        , _event(Event::newInstance())
    {}

    ErrorCode dispatch(Callback callback, void* userData)
    {
        return dispatch(g_main_context_default(), callback, userData);
    }

    ErrorCode dispatch(GMainContext* context, Callback callback, void* userData)
    {
        Guard guard(_mutex.get());

        _callback = callback;
        _rc = SPI_ERROR;
        _event->clear();
        _userData = userData;

        if (g_main_context_is_owner(context))
        {
            _logger.debug("already in context, calling directly");
            dispatchSource();
        }
        else
        {
            GSource *source = g_source_new(&sourceFuncs, sizeof(GSource));
            g_source_set_callback(source, dispatchSource, this, NULL);
            g_source_set_priority(source, G_PRIORITY_DEFAULT - 1);
            g_source_attach(source, context);

            const int waitCode = _event->wait(DISPATCH_TIMEOUT);
            if (waitCode != 0)
            {
                _logger.error("error waiting for dispatch: %d", waitCode);
                _rc = SPI_ERROR;
            }

            g_source_unref(source);
        }

        return _rc;
    }

private:

    static gboolean dispatchSource(gpointer data)
    {
        ((GlibMainLoopDispatcher*) data)->dispatchSource();
        return G_SOURCE_REMOVE;
    }

    void dispatchSource()
    {
        _rc = _callback(_userData);
        _event->signal();
    }

private:

    static const unsigned long DISPATCH_TIMEOUT = 30 * 1000;
    
    Logger _logger;
    Callback _callback;
    ErrorCode _rc;
    std::auto_ptr<Mutex> _mutex;
    std::auto_ptr<Event> _event;
    void* _userData;
};

}
}

#endif
