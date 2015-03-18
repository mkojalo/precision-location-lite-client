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

#ifndef WPS_CELL_ADAPTER_SPI_H_
#define WPS_CELL_ADAPTER_SPI_H_

#include <vector>
#include <string>

#include "spi/ScannedCellTower.h"
#include "spi/ErrorCodes.h"

namespace WPS {
namespace SPI {

/**
 * \addtogroup replaceable
 *
 * \b Telephony
 * \li \ref CellAdapter.h -- the interface to the Cell adapter
 */
/** @{ */

/**
 * Encapsulate a Cell adapter.
 *
 * @author Skyhook Wireless
 */
class CellAdapter
{
public:

    /**
     * The listener that receives events of the cell adapter.
     *
     * @see <code>setListener()</code>
     */
    class Listener
    {
    public:

        /**
         * Called when the scanned cell information has changed
         *
         * @param scannedCells the vector to contain the cell towers scanned
         */
        virtual void onCellChanged(const std::vector<ScannedCellTower>& scannedCells) =0;

        /**
         * Called when an error occurred
         *
         * @param code the <code>ErrorCode</code> of the error
         *             that occurred after the call to <code>startScan()</code>
         */
        virtual void onCellError(ErrorCode code) =0;
    };


    /**
     * @return a new instance of the \c CellAdapter.
     *
     * @note the caller is responsible for managing
     *       the lifecycle of the CellAdapter instances
     *       (<code>open()</code>, <code>close()</code>, and
     *       <code>~CellAdapter()</code>).
     */
    static CellAdapter* newInstance();

    /**
     * @note Implementation should call <code>close()</code>
     *       in overridden destructor
     */
    virtual ~CellAdapter()
    {}

    /**
     * @return a textual description of the adapter.
     *
     * @pre Until the instance has been <code>open()</code>'ed
     *      the description is undefined.
     */
    virtual std::string description() const =0;
    
    /**
     * Set the listener.
     *
     * @param listener receiver of the cell adapter events
     *
     * @note must be called before a first call to <code>startScan()</code>
     */
    virtual void setListener(Listener* listener) =0;

    /**
     * Connect to the underlying Cell adapter/driver
     * and start listening to cell information changes.
     *
     * @note Implementation should invoke <code>onCellChanged</code>
     *       immediately with the current cell information before returning.
     *
     * @note Calling <code>open()</code> on an already opened instance
     *       should be a no-op (and return <code>0</code>).
     *
     * @see <code>close()</code>
     */
    virtual ErrorCode open() =0;

    /**
     * Disconnects from the underlying Cell adapter/driver.
     *
     * @note Calling <code>close()</code> on an already closed instance should
     *       be a no-op.
     */
    virtual void close() =0;
    
    /**
     * Retrieve the IMEI of this adapter.
     *
     * @param imei the variable to store the imei
     * @return <code>0</code> if succeeded, an error code otherwise.
     */
    virtual ErrorCode getIMEI(std::string& imei) =0;

protected:

    CellAdapter()
    {}

private:

    /**
     * CellAdapter instances themselves cannot be copied.
     * Implementations may support copying.
     */
    CellAdapter(const CellAdapter&);
    CellAdapter& operator=(const CellAdapter&);
};

/** @} */

}
}

#endif
