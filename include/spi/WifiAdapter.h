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

#ifndef WPS_WIFI_ADAPTER_SPI_H_
#define WPS_WIFI_ADAPTER_SPI_H_

#include <vector>
#include <string>

#include "spi/ScannedAccessPoint.h"
#include "spi/ErrorCodes.h"

namespace WPS {
namespace SPI {

/**
 * \addtogroup replaceable
 *
 * WifiAdapter -- the interface to Wi-Fi adapter
 * \li \ref WifiAdapter.h
 */
/** @{ */

/**
 * Encapsulate a Wi-Fi adapter.
 *
 * @author Skyhook Wireless
 */
class WifiAdapter
{
public:

    /**
     * \ingroup nonreplaceable
     *
     * Power states
     *
     * @see <code>power()</code>
     */
    enum PowerState
    {
        FULL,
        POWER_SAVING
    };

    /**
     * The listener that receives events of the Wi-Fi adapter.
     *
     * @see <code>setListener()</code>
     */
    class Listener
    {
    public:

        /**
         * The listener routine called when the Wi-Fi scan has been completed successfully
         * after startScan() or when background scan is received.
         *
         * @param scannedAPs the vector to contain the access point scanned
         */
        virtual void onScanCompleted(const std::vector<ScannedAccessPoint>& scannedAPs) =0;

        /**
         * Called if an error occurred after a call to <code>startScan()</code>
         *
         * @param code the <code>ErrorCode</code> of the error
         *             that occurred after the call to <code>startScan()</code>
         */
        virtual void onScanFailed(ErrorCode code) =0;
    };

    /**
     * @return an available Wi-Fi adapter.
     *
     * @note The caller is responsible for managing
     *       the life-cycle of the WifiAdapter instances
     *       (<code>open()</code>, <code>close()</code>,
     *       <code>~WifiAdapter()</code>).
     */
    static WifiAdapter* newInstance();

    /**
     * @note Implementation should call <code>close()</code>
     *       in overridden destructor
     */
    virtual ~WifiAdapter()
    {}

    /**
     * Set the listener.
     *
     * @param listener receiver of the Wi-Fi adapter events
     *
     * @note must be called before a first call to <code>open()</code>
     */
    virtual void setListener(Listener* listener) =0;

    /**
     * @return a textual description of the adapter.
     *
     * @pre Until the instance has been <code>open()</code>'ed
     *      the description is undefined.
     */
    virtual std::string description() const =0;

    /**
     * Connect to the underlying Wi-Fi adapter/driver.
     *
     * @note Calling <code>open()</code> on an already opened instance
     *       should be a no-op (and return <code>0</code>).
     * @note Implementation should bring the adapter into power state
     *       that allows to initiate new Wi-Fi scans.
     *
     * @see <code>close()</code>
     */
    virtual ErrorCode open() =0;

    /**
     * Disconnects from the underlying Wi-Fi adapter/driver.
     *
     * @note Must guarantee that any pending callback will be called before returning.
     * @note Calling <code>close()</code> on an already closed instance should
     *       be a no-op.
     * @note Implementation should bring the adapter back
     *       to the original power state.
     */
    virtual void close() =0;

    /**
     * Start a new Wi-Fi scan for access points.
     *
     * @pre the instance must have been <code>open()</code>'ed prior to
     *      calling this method.
     *
     * @note If the Wi-Fi adapter represented by this instance
     *       is associated to an access point, calling this method
     *       must not result in breaking the association.
     *
     * @see <code>power()</code>
     */
    virtual void startScan() =0;

    /**
     * Retrieve the MAC of the associated access point.
     *
     * @param mac the variable to contain the MAC of the access point.
     *
     * @note If the adapter is not associated, this method should return SPI_ERROR_NOT_READY.
     */
    virtual ErrorCode getConnectedMAC(MAC& mac) =0;

    /**
     * Retrieve the MAC address of this adapter.
     *
     * @param mac the variable to contain the MAC of the adapter.
     */
    virtual ErrorCode getHardwareMAC(MAC& mac) =0;

    /**
     * Enter the specified power state.
     *
     * @param powerState power state to enter
     *
     * @note This method must not change the power state of the adapter
     *       if it was at full power when this instance was created.
     */
    virtual ErrorCode power(PowerState powerState) =0;

protected:

    WifiAdapter()
    {}

private:

    /**
     * WifiAdapter instances themselves cannot be copied.
     * Implementations may support copying.
     */
    WifiAdapter(const WifiAdapter&);
    WifiAdapter& operator=(const WifiAdapter&);
};

/** @} */

}
}

#endif
