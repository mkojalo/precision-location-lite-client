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

#ifndef _SKYHOOK_LITE_CLIENT_H_
#define _SKYHOOK_LITE_CLIENT_H_

/**
 * \cond
 */

#if __GNUC__ >= 4
#  define SHLC_EXPORT __attribute__((visibility("default")))
#else
#  define SHLC_EXPORT
#endif

/**
 * \endcond
 */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Return code.
 */
typedef enum
{
    /**
     * The call was successful.
     */
    SHLC_OK = 0,

    /**
     * No radio adapters were detected.
     */
    SHLC_ERROR_RADIO_NOT_AVAILABLE = 2,

    /**
     * No radio beacons in range.
     */
    SHLC_ERROR_NO_BEACONS_IN_RANGE = 3,

    /**
     * User authentication failed.
     */
    SHLC_ERROR_UNAUTHORIZED = 4,

    /**
     * The server is unavailable.
     */
    SHLC_ERROR_SERVER_UNAVAILABLE = 5,

    /**
     * A location couldn't be determined.
     */
    SHLC_ERROR_LOCATION_CANNOT_BE_DETERMINED = 6,

    /**
     * Network operation timed out.
     */
    SHLC_ERROR_TIMEOUT = 10,

    /**
     * Some other error occurred.
     */
    SHLC_ERROR = 99
} SHLC_ReturnCode;

/**
 * Calculated location type.
 */
typedef enum
{
    SHLC_LOCATION_TYPE_2D,
    SHLC_LOCATION_TYPE_3D
} SHLC_LocationType;

/**
 * Geographic location.
 */
typedef struct
{
    //@{
    /**
     * the calculated physical geographic location.
     */
    double latitude;
    double longitude;
    //@}

    /**
     * <em>horizontal positioning error</em> --
     * A calculated error estimate of the location result in meters.
     */
    double hpe;

    /**
     * The number of access-point used to calculate this location.
     */
    unsigned short nap;

    /**
     * A calculated estimate of speed in km/hr.
     *
     * A negative value is used to indicate an unknown speed.
     */
    double speed;

    /**
     * A calculated estimate of bearing as degree from north
     * clockwise (+90 is East).
     *
     * A negative value is used to indicate an unknown bearing.
     */
    double bearing;

    /**
     * The number of cell tower used to calculate this location.
     */
    unsigned short ncell;

    /**
     * The number of unique location area codes used to calculate this location.
     */
    unsigned short nlac;

    /**
     * The number of satellite used to calculate this location.
     */
    unsigned short nsat;

    /**
     * A calculated altitude above mean sea level in meters.
     */
    double altitude;

    /**
     * Type of calculated location.
     */
    SHLC_LocationType type;

    /**
     * Number of milliseconds elapsed since the time the
     * location was calculated.
     */
    unsigned long age;
} SHLC_Location;

/**
 * Return a string containing the version information
 * as <code>&lt;major&gt;.&lt;minor&gt;.&lt;revision&gt;.&lt;build&gt;</code>
 *
 * \return the version information
 */
SHLC_EXPORT const char*
SHLC_version();

/**
 * Initialize the Skyhook Lite Client library.
 * \n
 * Must be called once before making any other calls,
 * e.g. on application startup.
 *
 * \return an opaque handle to be passed to SHLC API calls
 *         or \c NULL if an error occurred.
 */
SHLC_EXPORT void*
SHLC_init();

/**
 * Deinitialize the Skyhook Lite Client library.
 * \n
 * Must be called once to free resources held by the library
 * when it is no longer in use, e.g. on application shutdown.
 *
 * \param handle handle value returned by \c SHLC_init().
 */
SHLC_EXPORT void
SHLC_deinit(const void* handle);

/**
 * Request geographic location based on observed Wi-Fi access points,
 * cell towers, and GPS signals.
 *
 * \param handle handle value returned by \c SHLC_init().
 * \param key user's API key.
 * \param location pointer to return a \c SHLC_Location object.
 *                 \n
 *                 This pointer must be freed by calling \c SHLC_free_location().
 *
 * \return a \c SHLC_ReturnCode
 */
SHLC_EXPORT SHLC_ReturnCode
SHLC_location(const void* handle,
              const char* key,
              SHLC_Location** location);

/**
 * Free a \c SHLC_Location object returned by \c SHLC_location().
 *
 * \param handle handle value returned by \c SHLC_init().
 * \param location location object to free.
 */
SHLC_EXPORT void
SHLC_free_location(const void* handle,
                   SHLC_Location* location);

/**
 * \endcond
 */

#ifdef __cplusplus
}
#endif

#endif // _SKYHOOK_LITE_CLIENT_H_
