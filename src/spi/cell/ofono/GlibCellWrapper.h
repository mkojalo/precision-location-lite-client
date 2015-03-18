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

#ifndef WPS_SPI_GLIB_CELL_WRAPPER_H_
#define WPS_SPI_GLIB_CELL_WRAPPER_H_

#include "spi/CellAdapter.h"
#include "spi/Thread.h"
#include "spi/Concurrent.h"

#include "glib/GlibMainLoopDispatcher.h"
#include "glib/GlibMainLoopThread.h"

#include <dbus/dbus-glib.h>
#include <pthread.h>

namespace WPS {
namespace SPI {

class GlibCellWrapper
    : public CellAdapter
{
public:

    GlibCellWrapper(CellAdapter* adapter)
        : _logger("WPS.SPI.GlibCellWrapper")
        , _isOpen(false)
        , _cellAdapter(adapter)
    {}

    ~GlibCellWrapper()
    {
        close();
    }

    std::string description() const
    {
        return _cellAdapter->description();
    }

    ErrorCode open()
    {
        if (_isOpen)
            return SPI_OK;

        _logger.debug("starting glib main loop thread");

        if (_mainLoopThread.start() != SPI_OK)
            return SPI_ERROR;

        _logger.debug("opening adapter");

        ErrorCode rc =
            _mainLoopDispatcher.dispatch(_mainLoopThread.getContext(),
                                         dispatchableOpen,
                                         this);
        if (rc != SPI_OK)
        {
            _logger.error("failed to open adapter (%d)", rc);
            _mainLoopThread.stop();
            return rc;
        }

        _isOpen = true;

        _logger.debug("opened");
        return SPI_OK;
    }

    void close()
    {
        if (! _isOpen)
            return;

        _logger.debug("closing adapter");

        _mainLoopDispatcher.dispatch(_mainLoopThread.getContext(),
                                     dispatchableClose,
                                     this);

        _logger.debug("stopping glib main loop thread");

        _mainLoopThread.stop();
        _isOpen = false;

        _logger.debug("closed");
    }

    void setListener(Listener* listener)
    {
        _cellAdapter->setListener(listener);
    }

    ErrorCode getIMEI(std::string& imei)
    {
        _imei = &imei;

        return _mainLoopDispatcher.dispatch(
            _mainLoopThread.getContext(),
            dispatchableGetIMEI,
            this);
    }

private:

    static GlibCellWrapper* getSelf(void* userData)
    {
        return static_cast<GlibCellWrapper*>(userData);
    }

    static ErrorCode dispatchableOpen(void* userData)
    {
        return getSelf(userData)->_cellAdapter->open();
    }

    static ErrorCode dispatchableClose(void* userData)
    {
        getSelf(userData)->_cellAdapter->close();
        return SPI_OK;
    }

    static ErrorCode dispatchableGetIMEI(void* userData)
    {
        return getSelf(userData)->_cellAdapter->getIMEI(*getSelf(userData)->_imei);
    }

private:

    Logger _logger;
    bool _isOpen;

    GlibMainLoopDispatcher _mainLoopDispatcher;
    GlibMainLoopThread _mainLoopThread;

    std::auto_ptr<CellAdapter> _cellAdapter;

    std::string* _imei;
    Listener* _listener;
};

}
}

#endif

