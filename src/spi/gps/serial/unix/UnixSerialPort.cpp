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

#include "spi/Logger.h"
#include "spi/Concurrent.h"

#include "../SerialPort.h"

#include <sys/select.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>

#include <fcntl.h>
#include <errno.h>
#include <termios.h>
#include <unistd.h>
#include <pthread.h>

#include <memory>
#include <cstring>
#include <string>

namespace WPS {
namespace SPI {

/**********************************************************************/
/*                                                                    */
/* UnixSerialPort                                                     */
/*                                                                    */
/**********************************************************************/

class UnixSerialPort
    : public SerialPort
{
public:

    UnixSerialPort(const std::string& id, const int port, const int (&pipe)[2])
        : _logger("WPS.SPI.UnixSerialPort"),
          _id(id),
          _port(port),
          _pipeRead(pipe[0]),
          _pipeWrite(pipe[1]),
          _listener(NULL),
          _started(false),
          _mutex(Mutex::newInstance())
    {}

    ~UnixSerialPort()
    {
        stop();
        ::close(_port);
        // TODO: restore initial port settings?
        _logger.debug("closed successfully");
    }

    bool start(Listener* listener)
    {
        if (_started)
        {
            _logger.error("already started");
            return false;
        }

        _listener = listener;

        int rc = pthread_create(&_readThread, NULL, readThreadCallback, static_cast<void*>(this));
        if (rc != 0)
        {
            _logger.error("pthread_create failed (%d)", rc);
            return false;
        }

        _logger.debug("created read thread");

        _started = true;
        return true;
    }

    void stop()
    {
        if (! _started)
            return;

        _logger.debug("closing read thread");

        char c = 'a';
        ::write(_pipeWrite, &c, 1);

        pthread_join(_readThread, NULL);

        _listener = NULL;
        _started = false;
    }

    bool setBaudRate(int baudRate)
    {
        if (_logger.isDebugEnabled())
            _logger.debug("setting baud rate to %d", baudRate);

        // Purge
        if (tcflush(_port, TCIOFLUSH) == -1)
        {
            _logger.error("tcflush failed (%d)", errno);
            return false;
        }

        // Set baudrate and switch port to raw mode
        struct termios settings;
        if (tcgetattr(_port, &settings) == -1)
        {
            _logger.error("tcgetattr failed (%d)", errno);
            return false;
        }

        bzero(&settings, sizeof(settings));

        cfsetospeed(&settings, toTermIOBaubrate(baudRate));

        settings.c_cflag |= (CLOCAL | CREAD);
        settings.c_cflag &= ~PARENB;
        settings.c_cflag &= ~CSTOPB;
        settings.c_cflag |= CS8;
        settings.c_cflag |= CRTSCTS;
        settings.c_oflag |= (OPOST | ONLCR);

        if (tcsetattr(_port, TCSAFLUSH, &settings) == -1)
        {
            _logger.error("tcsetattr failed (%d)", errno);
            return false;
        }

        // Switch to blocking mode
        int flags = fcntl(_port, F_GETFL);
        if (flags == -1)
        {
            _logger.error("fcntl(F_GETFL) failed (%d)", errno);
            return false;
        }

        if (fcntl(_port, F_SETFL, flags & ~O_NONBLOCK) == -1)
        {
            _logger.error("fcntl(F_SETFL) failed (%d)", errno);
            return false;
        }

        _logger.debug("baud rate set successfully");
        return true;
    }

    int getBaudRate() const
    {
        struct termios settings;
        if (tcgetattr(_port, &settings) == -1)
        {
            _logger.error("tcgetattr failed (%d)", errno);
            return -1;
        }

        return toIntBaubrate(cfgetospeed(&settings));
    }


    bool setTimeout(int milliseconds)
    {
        // Not implemented
        return false;
    }

    int getTimeout()
    {
        // Not implemented
        return -1;
    }

    const std::string& id() const
    {
        return _id;
    }

private:

    static void* readThreadCallback(void* arg)
    {
        UnixSerialPort* _this = reinterpret_cast<UnixSerialPort*>(arg);
        _this->readThread();
        return 0;
    }

    void readThread()
    {
        _listener->onStarting(this);

        fd_set readFds;
        FD_ZERO(&readFds);

        do
        {
            FD_SET(_pipeRead, &readFds);
            FD_SET(_port, &readFds);

            int rc = ::select(std::max(_pipeRead, _port) + 1, &readFds, NULL, NULL, NULL);
            if (rc == -1)
            {
                _logger.error("select failed (%d)", errno);
                break;
            }
            if (rc == 0)
            {
                _logger.debug("select timed out");
                // TODO: timeout
            }
            else
            {
                if (FD_ISSET(_pipeRead, &readFds))
                {
                    _logger.debug("read thread signalled to stop");
                    char c;
                    ::read(_pipeRead, &c, 1);
                    break;
                }
                else if (FD_ISSET(_port, &readFds))
                {
                    char c;
                    const int bytesRead = ::read(_port, &c, 1);
                    if (bytesRead == 0)
                    {
                        _logger.warn("eof from port");
                    }
                    else if (bytesRead < 0)
                    {
                        _logger.error("error reading port %d (errno %d)", bytesRead, errno);
                        break;
                    }
                    else if (! _listener->onData(this, &c, 1))
                    {
                        _logger.debug("listener requested to stop");
                        break;
                    }
                }
            }
        }
        while (true);

        _listener->onStopping(this);
        _logger.debug("stopped");
    }

    static speed_t toTermIOBaubrate(unsigned short baudrate)
    {
        switch (baudrate)
        {
            case 1200: return B1200;
            case 2400: return B2400;
            case 4800: return B4800;
            case 9600: return B9600;
            case 19200: return B19200;
            case 38400: return B38400;
            case 57600: return B57600;
            default:
                return B57600;
        }
    }

    static unsigned short toIntBaubrate(speed_t baudrate)
    {
        switch (baudrate)
        {
            case B1200: return 1200;
            case B2400: return 2400;
            case B4800: return 4800;
            case B9600: return 9600;
            case B19200: return 19200;
            case B38400: return 38400;
            case B57600: return 57600;
            default:
                return 57600;
        }
    }

private:

    Logger _logger;

    const std::string _id;
    const int _port;
    const int _pipeRead, _pipeWrite;

    Listener* _listener;
    pthread_t _readThread;
    bool _started;
    std::auto_ptr<Mutex> _mutex;
};

/**********************************************************************/
/*                                                                    */
/* SerialPort factories                                               */
/*                                                                    */
/**********************************************************************/

SerialPort*
SerialPort::getById(const std::string& id)
{
    const int port = ::open(id.c_str(), O_NOCTTY | FNDELAY | O_RDONLY);

    if (port == -1)
    {
        Logger logger("WPS.SPI.SerialPort");
        logger.error("failed to open port %s (%d)", id.c_str(), errno);
        return NULL;
    }

    int pipeFd[2];
    if (::pipe(pipeFd))
    {
        Logger logger("WPS.SPI.SerialPort");
        logger.error("failed to create pipe (%d)", errno);
        ::close(port);
        return NULL;
    }

    SerialPort* const serialPort = new UnixSerialPort(id, port, pipeFd);
    if (! serialPort->setBaudRate(4800))
    {
        delete serialPort;
        return NULL;
    }

    return serialPort;
}

std::list<SerialPort*>
SerialPort::enumerate()
{
    // Not implemented
    return std::list<SerialPort*>();
}

}
}
