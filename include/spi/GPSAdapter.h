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

#ifndef WPS_SPI_GPS_ADAPTER_H_
#define WPS_SPI_GPS_ADAPTER_H_

#include <vector>
#include <string>

#include "spi/GPSData.h"
#include "spi/ErrorCodes.h"

namespace WPS {
namespace SPI {

/**
 * \addtogroup replaceable
 *
 * \b GPS
 * \li \ref GPSAdapter.h
 */
/** @{ */

/**
 * Encapsulates a GPS adapter.
 *
 * @author Skyhook Wireless
 */
class GPSAdapter
{
public:

    /**
     * The listener that receives events of the gps adapter.
     *
     * @see <code>setListener()</code>
     */
    class Listener
    {
    public:

        /**
         * Called when gps data has been received
         * 
         * @param gpsData the variable to contain the GPS data
         */
        virtual void onGpsData(const GPSData& gpsData) =0;

        /**
         * Called when an error occurred
         *
         * @param code the <code>ErrorCode</code> of the error
         */
        virtual void onGpsError(ErrorCode code) =0;
    };

    /**
     * @return a new instance of the \c GPSAdapter.
     *
     * @note the caller is responsible for managing
     *       the life-cycle of the GPSAdapter instances
     *       (<code>open()</code>, <code>close()</code>,
     *       <code>~GPSAdapter()</code>).
     */
    static GPSAdapter* newInstance();

    /**
     * @note Implementation should call <code>close()</code>
     *       in overridden destructor
     */
    virtual ~GPSAdapter()
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
     * @param listener receiver of the gps adapter events
     *
     * @note must be called before a first call to <code>open()</code>
     */
    virtual void setListener(Listener* listener) =0;

    /**
     * Connect to the underlying GPS adapter/driver
     * and start location tracking.
     *
     * @note Calling <code>open()</code> on an already opened instance
     *       should be a no-op (and return <code>0</code>).
     *
     * @pre Must be preceded by a call to <code>setListener()</code>.
     *
     * @see <code>close()</code>
     */
    virtual ErrorCode open() =0;

    /**
     * Disconnect from the underlying GPS adapter/driver
     * and stop location tracking.
     *
     * @note Calling <code>close()</code> on an already closed instance should
     *       be a no-op.
     */
    virtual void close() =0;

protected:

    GPSAdapter()
    {}

private:

    /**
     * GPSAdapter instances themselves cannot be copied.
     * Implementations may support copying.
     */
    GPSAdapter(const GPSAdapter&);
    GPSAdapter& operator=(const GPSAdapter&);
};

/** @} */

}
}

#endif
