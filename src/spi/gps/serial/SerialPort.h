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

#pragma once

#include <list>
#include <string>

namespace WPS {
namespace SPI {

/**
 * An asynchronous serial port interface
 *
 * A SerialPort object represents an opened serial device
 */
class SerialPort
{
public:

    /**
       Callback class
     */
    class Listener
    {
    public:

        /**
           Reading is being started
           @param port port instance
         */
        virtual void onStarting(SerialPort* port)
        {}

        /**
           Reading is being stopped
           @param port port instance
         */
        virtual void onStopping(SerialPort* port)
        {}

        /**
           New data has arrived
           @param port port instance
           @param data new data buffer
           @param size arrived data size
           @return true if the SerialPort should continue reading
         */
        virtual bool onData(SerialPort* port,
                            const char* data,
                            unsigned int size) = 0;

        /**
           Port reading has timed out
           @param port port instance
           @return true if the SerialPort should continue reading
         */
        virtual bool onTimeout(SerialPort* port)
        {
            return true;
        }

        /**
           Read error has occured
           @param port port instance
           @return true if the SerialPort should continue reading
         */
        virtual bool onError(SerialPort* port)
        {
            return false;
        }
    };

    virtual ~SerialPort()
    {}

    /**
       Start reading the serial port device
       @param listener state handler
       @return true if started
     */
    virtual bool start(Listener* listener) = 0;

    /**
       Asyncronously stop reading the serial port device.
       @note this method cannot be called from the callbacks
     */
    virtual void stop() = 0;

    /**
       Change serial device baudrate
       @param baudrate baudrate as integer (e.g. 4800)
       @note baudrate setting persists across start/stop cycles
    */
    virtual bool setBaudRate(int baudRate) = 0;

    /**
       Get serial device baudrate
       @return baudrate value as integer of -1 if an error occured
    */
    virtual int getBaudRate() const = 0;

    /**
       Change serial port reading timeout
       @param milliseconds timeout value in ms
       @note timeout setting persists across start/stop cycles
    */
    virtual bool setTimeout(int milliseconds) = 0;

    /**
       Get serial port reading timeout
       @return milliseconds timeout value in ms
    */
    virtual int getTimeout() = 0;

    /**
       Get platform specific unique device id of the port.
    */
    virtual const std::string& id() const = 0;

    /**
       Create a new instance by a device id
       @param id platform specific unique device id
       @return dynamically allocated SerialPort instance or
               NULL if the device with the passed id cannot be opened
    */
    static SerialPort* getById(const std::string& id);

    /**
       Enumerate readable serial devices
       @return list of dynamically allocated SerialPort instances
     */
    static std::list<SerialPort*> enumerate();
};

}
}
