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

#ifndef WPS_SPI_LOGGER_H_
#define WPS_SPI_LOGGER_H_

#include <cstdarg>
#include <string>

namespace WPS {
namespace SPI {

/**
 * \addtogroup replaceable
 *
 * \b Logging
 * \li \ref Logger
 */

/**
 * \ingroup nonreplaceable
 *
 * Logger is the interface to the logging system.
 * Logger delegates its calls to Impl.
 *
 * @par Example:
 * @code
 * Logger logger(__FILE__);
 * logger.info("this is a simple message");
 * if (logger.isEnabledFor(Logger::LEVEL_INFO))
 *     logger.log(Logger::LEVEL_INFO, "this is %s message", "another");
 * @endcode
 *
 * @note \c Logger instances should \b not be static in order to prevent
 *       initialization order problem.
 *
 * @see Impl for actual implementation of this class.
 *
 * @author Skyhook Wireless
 */
class Logger
{
public:

    /**
     * @param category the name of the category to send messages to
     */
    Logger(const char* category)
        : _category(category)
        , _impl(Impl::getInstance())
    {}

    ~Logger()
    {}

    enum Level
    {
        LEVEL_OFF       = -1,

        /**
         * system is unusable
         */
        LEVEL_FATAL     = 0,

        /**
         * action must be taken immediately
         */
        LEVEL_ALERT     = 1,

        /**
         * critical condition
         */
        LEVEL_CRITICAL  = 2,

        /**
         * error condition
         */
        LEVEL_ERROR     = 3,

        /**
         * warning condition
         */
        LEVEL_WARN      = 4,

        /**
         * important messages
         */
        LEVEL_NOTICE    = 5,

        /**
         * informational messages
         */
        LEVEL_INFO      = 6,

        /**
         * debug level messages
         */
        LEVEL_DEBUG     = 7,

        LEVEL_ON        = 8
    };

    /**
     * @return <code>true</code> if a message sent to this Logger
     *         is actually written to a destination.
     */
    bool isEnabledFor(Level level) const
    {
        return _impl.isEnabledFor(_category, level);
    }

    /**
     * log a formatted message.
     *
     * @see printf
     * @see vprintf
     */
     // @{
    void log(Level level, const char* format, ...) const
#ifdef __GNUC__
        __attribute__ ((__format__ (__printf__, 3, 4)))
#endif
    ;

    void log(Level level, const char* format, std::va_list ap) const;
    // @}

    /**
     * Convenience method for:
     * @code
     *   isEnabledFor(LEVEL_FATAL);
     * @endcode
     */
    bool isFatalEnabled() const
    {
        return isEnabledFor(LEVEL_FATAL);
    }

    /**
     * Convenience method for:
     * @code
     *   log(LEVEL_FATAL, format, ...)
     * @endcode
     */
    void fatal(const char* format, ...) const
#ifdef __GNUC__
        __attribute__ ((format (printf, 2, 3)))
#endif
    ;

    /**
     * Convenience method for:
     * @code
     *   isEnabledFor(LEVEL_ALERT);
     * @endcode
     */
    bool isAlertEnabled() const
    {
        return isEnabledFor(LEVEL_ALERT);
    }

    /**
     * Convenience method for:
     * @code
     *   log(LEVEL_ALERT, format, ...)
     * @endcode
     */
    void alert(const char* format, ...) const
#ifdef __GNUC__
        __attribute__ ((format (printf, 2, 3)))
#endif
    ;

    /**
     * Convenience method for:
     * @code
     *   isEnabledFor(LEVEL_CRITICAL);
     * @endcode
     */
    bool isCriticalEnabled() const
    {
        return isEnabledFor(LEVEL_CRITICAL);
    }

    /**
     * Convenience method for:
     * @code
     *   log(LEVEL_CRITICAL, format, ...)
     * @endcode
     */
    void critical(const char* format, ...) const
#ifdef __GNUC__
        __attribute__ ((format (printf, 2, 3)))
#endif
    ;

    /**
     * Convenience method for:
     * @code
     *   isEnabledFor(LEVEL_ERROR);
     * @endcode
     */
    bool isErrorEnabled() const
    {
        return isEnabledFor(LEVEL_ERROR);
    }

    /**
     * Convenience method for:
     * @code
     *   log(LEVEL_ERROR, format, ...)
     * @endcode
     */
    void error(const char* format, ...) const
#ifdef __GNUC__
        __attribute__ ((format (printf, 2, 3)))
#endif
    ;

    /**
     * Convenience method for:
     * @code
     *   isEnabledFor(LEVEL_WARN);
     * @endcode
     */
    bool isWarnEnabled() const
    {
        return isEnabledFor(LEVEL_WARN);
    }

    /**
     * Convenience method for:
     * @code
     *   log(LEVEL_WARN, format, ...)
     * @endcode
     */
    void warn(const char* format, ...) const
#ifdef __GNUC__
        __attribute__ ((format (printf, 2, 3)))
#endif
    ;

    /**
     * Convenience method for:
     * @code
     *   isEnabledFor(LEVEL_NOTICE);
     * @endcode
     */
    bool isNoticeEnabled() const
    {
        return isEnabledFor(LEVEL_NOTICE);
    }

    /**
     * Convenience method for:
     * @code
     *   log(LEVEL_NOTICE, format, ...)
     * @endcode
     */
    void notice(const char* format, ...) const
#ifdef __GNUC__
        __attribute__ ((format (printf, 2, 3)))
#endif
    ;

    /**
     * Convenience method for:
     * @code
     *   isEnabledFor(LEVEL_INFO);
     * @endcode
     */
    bool isInfoEnabled() const
    {
        return isEnabledFor(LEVEL_INFO);
    }

    /**
     * Convenience method for:
     * @code
     *   log(LEVEL_INFO, format, ...)
     * @endcode
     */
    void info(const char* format, ...) const
#ifdef __GNUC__
        __attribute__ ((format (printf, 2, 3)))
#endif
    ;

    /**
     * Convenience method for:
     * @code
     *   isEnabledFor(LEVEL_DEBUG);
     * @endcode
     */
    bool isDebugEnabled() const
    {
        return isEnabledFor(LEVEL_DEBUG);
    }

    /**
     * Convenience method for:
     * @code
     *   log(LEVEL_DEBUG, format, ...)
     * @endcode
     */
    void debug(const char* format, ...) const
#ifdef __GNUC__
        __attribute__ ((format (printf, 2, 3)))
#endif
    ;

public:

    /**
     * \ingroup replaceable
     *
     * Actual replaceable implementation of Logger.
     *
     * @author Skyhook Wireless
     */
    struct Impl
    {
        /**
         * @see Logger::Logger(const char*)
         */
        static Impl& getInstance();

        virtual ~Impl()
        {}

        /**
         * @see Logger::isEnabledFor()
         */
        virtual bool isEnabledFor(const char* category, Level) const =0;

        /**
         * @see Logger::log(Level, const char*, ...)
         * @see Logger::log(Level, const char*, std::va_list)
         */
        virtual void log(const char* category,
                         Level level,
                         const char* format,
                         std::va_list ap) =0;

    protected:

        /**
         * Convenience function to convert a <code>Logger::Level</code> to a <code>string</code>.
         */
        static const char* toString(Level level);

        /**
         * Convenience function to format the list of arguments (<code>ap</code>)
         * according to <code>format</code>.
         *
         * @see vsnprintf
         */
        static std::string format(const char* format, std::va_list ap);
    };

private:

    // not implemented
    Logger(const Logger& rhs);
    Logger& operator=(const Logger& rhs);

private:

    const char* _category;
    Impl& _impl;
};

}
}

#endif
