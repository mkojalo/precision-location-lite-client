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

#ifndef WPS_SPI_SCANNED_GSM_TOWER_H_
#define WPS_SPI_SCANNED_GSM_TOWER_H_

#include <functional>
#include <string>
#include <vector>

#include "spi/CellTower.h"
#include "spi/Time.h"
#include "spi/Assert.h"

namespace WPS {
namespace SPI {

/**
 * \ingroup nonreplaceable
 *
 * Encapsulate a scanned cell tower, made of a cell tower,
 * a power reading (RSSI - Received Signal Strength Indication),
 * a timing advance, and a timestamp.
 *
 * @see http://en.wikipedia.org/wiki/Rssi
 *
 * @since WPS API 2.7
 *
 * @author Skyhook Wireless
 */
class ScannedCellTower
{
public:

    /**
     * Creates a new instance.
     * <br>
     * The timestamp of this reading is defaulted to the current time.
     *
     * @param cell Cell tower to copy data from
     * @param timing Timing Advance
     * @param rssi the received signal strength in dBm.
     */
    ScannedCellTower(const CellTower& cell,
                     int timing,
                     short rssi)
        : _cell(cell)
        , _timing(timing)
        , _rssi(rssi)
    {
        assert(-255 <= _rssi && _rssi <= 0);
    }

    /**
     * Creates a new instance.
     *
     * @param cell Cell tower to copy data from
     * @param timing Timing Advance
     * @param rssi the received signal strength in dBm.
     * @param timestamp the time when the reading was captured.
     */
    ScannedCellTower(const CellTower& cell,
                     int timing,
                     short rssi,
                     const Timer& timestamp)
        : _cell(cell)
        , _timing(timing)
        , _rssi(rssi)
        , _timestamp(timestamp)
    {
        assert(-255 <= _rssi && _rssi <= 0);
    }

    ScannedCellTower(const ScannedCellTower& rhs)
        : _cell(rhs._cell)
        , _timing(rhs._timing)
        , _rssi(rhs._rssi)
        , _timestamp(rhs._timestamp)
    {}

    ScannedCellTower& operator=(const ScannedCellTower& rhs)
    {
        if (this != &rhs)
        {
            _cell = rhs._cell;
            _timing = rhs._timing;
            _rssi = rhs._rssi;
            _timestamp = rhs._timestamp;
        }
        return *this;
    }

    ~ScannedCellTower()
    {}

    int compare(const ScannedCellTower& that) const
    {
        // In order to keep the newest measurements when making the list of
        // ScannedCellTowers unique, it is important that this compares the
        // CellTower first and then the age.

        if (const int r = _cell.compare(that._cell))
            return r;

        // Make newer readings sort first
        if (const int r = _timestamp.compare(that._timestamp))
            return r;

        // IMPORTANT: It is safe to subtract the values for _rssi and _timing
        //            because _rssi is a short and _timing takes only small
        //            values. If the range for them is ever changed, then it
        //            needs to be reexamined whether subtraction can still
        //            be used.

        if (const int r = _rssi - that._rssi)
            return r;

        return _timing - that._timing;
    }

    const CellTower& getCell() const
    {
        return _cell;
    }

    int getTimingAdvance() const
    {
        return _timing;
    }

    short getRssi() const
    {
        return _rssi;
    }

    const Timer& getTimestamp() const
    {
        return _timestamp;
    }

    /**
     * @return <code>this</code> scanned GSM tower as a string.
     */
    std::string toString() const
    {
        return getCell().toString()
            + "," + itoa(_rssi)
            + "," + _timestamp.toString();
    }

    struct CellSame
        : std::binary_function<ScannedCellTower, ScannedCellTower, bool>
    {
        bool operator()(const ScannedCellTower& lhs,
                        const ScannedCellTower& rhs) const
        {
            return lhs.getCell().compare(rhs.getCell()) == 0;
        }
    };

    struct CellLess
        : std::binary_function<ScannedCellTower, ScannedCellTower, bool>
    {
        bool operator()(const ScannedCellTower& lhs,
                        const ScannedCellTower& rhs) const
        {
            return lhs.getCell().compare(rhs.getCell()) < 0;
        }
    };

    struct CellEqualsTo
        : std::binary_function<ScannedCellTower, CellTower, bool>
    {
        bool operator()(const ScannedCellTower& scannedCell,
                        const CellTower& cell) const
        {
            return scannedCell.getCell() == cell;
        }
    };

    struct LacSame
        : std::binary_function<ScannedCellTower, ScannedCellTower, bool>
    {
        bool operator()(const ScannedCellTower& lhs,
                        const ScannedCellTower& rhs) const
        {
            return lhs.getCell().getLac() == rhs.getCell().getLac();
        }
    };

    struct LacLess
        : std::binary_function<ScannedCellTower, ScannedCellTower, bool>
    {
        bool operator()(const ScannedCellTower& lhs,
                        const ScannedCellTower& rhs) const
        {
            return lhs.getCell().getLac() < rhs.getCell().getLac();
        }
    };

    struct HasNoLac
        : std::unary_function<ScannedCellTower, bool>
    {
        bool operator()(const ScannedCellTower& arg) const
        {
            return ! arg.getCell().hasLac();
        }
    };

    struct IsNotGsmOrUmts
        : std::unary_function<ScannedCellTower, bool>
    {
        bool operator()(const ScannedCellTower& arg) const
        {
            return arg.getCell().getType() != CellTower::GSM
                       && arg.getCell().getType() != CellTower::UMTS;
        }
    };

    struct IsUmts
        : std::unary_function<ScannedCellTower, bool>
    {
        bool operator()(const ScannedCellTower& arg) const
        {
            return arg.getCell().getType() == CellTower::UMTS;
        }
    };

private:

    CellTower _cell;
    int _timing;
    short _rssi;
    Timer _timestamp;
};

}
}

#endif
