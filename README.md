# Precision Location Lite Client Integration Guide

## Overview

Precision Location Lite Client is location awareness, in a lightweight package. It's written in C++ and deployed as a dynamic library, with a C API exposed to applications.

The following project acts as an example of how to implement a location-based solution using Skyhook's positioning, and can be used as a starting point for your application.

The Skyhook Precision Location Lite Client provides a pluggable software architecture that can easily be ported to new platforms and/or devices. The source code below includes an implementation guide for the Linux platform that serves as a starting example.

## Requirements

### Build and run now

* Linux
* CMake 2.6 or higher
* GCC 3.4 (or higher) or Clang 3.3 (or higher)
* XML: libxml2
* HTTPS: libcurl, and openssl or gnutls
* Wi-Fi: nl80211
* Cell: oFono API to enable cell positioning
* GPS: NMEA or SiRF-compatible GPS receiver with serial interface

### Extending to a non-Linux based platform

* С++98 compatible toolchain
* STL (streams support not required)
* CMake 2.6+ recommended but not required

## API Key

Before you proceed to building your project, first make sure to acquire an API key at `my.skyhookwireless.com`. Do not share this secret key. 

To do this, follow these steps:
* Create a My.Skyhook account if you don't already have one by going to [my.skyhookwireless.com/register](https://my.skyhookwireless.com/register).
* Create a new Precision Location project. Select Linux for your project's OS at this time. Once created, note your API key. You can get back to this any time via [my.skyhookwireless.com/dashboard](https://my.skyhookwireless.com/dashboard).

## Building

Below are the steps required to build a Precision Location Lite Client project on Ubuntu/Linux.

Install the C++ toolchain (gcc/g++, make, etc), CMake and the required libraries:
```
sudo apt-get install build-essential cmake pkg-config
sudo apt-get install libxml2-dev libcurl4-openssl-dev
sudo apt-get install libnl-3-dev libnl-genl-3-dev libnl-route-3-dev
```

Check out the project to `/home/user/skyhook/liteclient` and create a build directory at `/home/user/skyhook/cmake_build`.

Your directory structure should look like this:
```
/skyhook
    /cmake_build
    /liteclient
```

Now you can build the project with the following commands:
```
cd /home/user/skyhook/cmake_build
cmake /home/user/skyhook/liteclient -DSKYHOOK_API_KEY=<YOUR_API_KEY>
make
```

As a result, you should see `libskyhookliteclient.so` and `skyhooklitetest` in the current directory. You can now run:
```
sudo ./skyhooklitetest
```

## Structure

The Precision Location Lite Client provides a pluggable software library that can easily be ported to new platforms and/or devices. The source tree is split into two layers:

| | Purpose | Location | C++ Namespace |
| --- | --- | --- | --- |
| API | Implementation of SHLC API | include/api src/api | WPS::API |
| SPI | Platform abstraction (Service Provider Interface) | include/spi src/spi | WPS::SPI |

The API layer implements the generic platform independent of SHLC API logic. It is built on top of SPI, and creates the `libskyhookliteclient.so` target.

The SPI layer provides a set of primitives dependent of platform specifics (thread, time, Wi-Fi, GPS etc.) defined as C++ classes or functions (listed under `include/spi`). The SPI layer creates a set of static library targets: `libwpsspi-thread.a`, `libwpsspi-wifi.a`, `libwpsspi-gps.a` and so on.

Below is a diagram that visualizes all the above:

![structure](https://cloud.githubusercontent.com/assets/8643222/6698009/1209beac-cccb-11e4-99c5-3d3f1694dfde.png)

## Build Tree

According to the project structure described above, you can find a `CMakeLists.txt` file for each library target in the build tree.

When you issue a `cmake` command (see [Building](#building)), the root `CMakeLists.txt` file will be loaded, which in turn will load the tree of all dependent targets, as shown below:

![tree](https://cloud.githubusercontent.com/assets/8643222/6698010/13ff2a08-cccb-11e4-8d66-7d89c706f76e.png)

## Customization

The key part of the SPI layer structure is that each SPI target may have multiple implementations that you can switch at build time. This is achieved by using a CMake variable pointing to a subdirectory.

Let's consider the Wi-Fi target as an example:
```
src/
    spi/
    wifi/
        /nl80211
        /static
    CMakeLists.txt
    MAC.cpp
```

You can see that the `wifi` directory has 2 subdirectories:

|Subdirectory|Description|
| --- | --- |
| nl80211 | implementation based on the [802.11 netlink interface](http://wireless.kernel.org/en/developers/Documentation/nl80211) |
| static | implementation that simulates a hard coded Wi-Fi scan (for testing) |

The `src/spi/wifi/CMakeLists.txt` defines the `WPS_SPI_WIFI_ADAPTER` variable pointing to one of the two values.

The default value is obviously `nl80211`, but you can customize the build and choose `static` wifi (for example you want to test on a device prototype that lacks wifi hardware):
```
-DWPS_SPI_WIFI_ADAPTER=static
```

The same applies to each individual SPI target. Examine the corresponding `CMakeLists.txt` file to see the configuration options for each.

### GPS configuration

As indicated in the previous section, you can choose a GPS implementation (similar to Wi-Fi) by specifying `WPS_SPI_GPS_ADAPTER`. Possible options are `unixserial`, `static` or `null`.

GPS is disabled (set to `null`) by default, but can be enabled by choosing the `unixserial` implementation. You will have to configure additional parameters for your GPS device defined in `src/spi/gps/unix/CMakeLists.txt`:

|Parameter|Description|
| --- | --- |
| WPS_SPI_GPS_ADAPTER | implementation: `unixserial`, `static` or `null` |
| WPS_SPI_GPS_DEVICE | name of serial device to connect |
| WPS_SPI_GPS_PROTOCOL | protocol name: `nmea` or `sirf` |

For example:
```
-DWPS_SPI_GPS_ADAPTER=unixserial
-DWPS_SPI_GPS_DEVICE=/dev/ttyUSB0
-DWPS_SPI_GPS_PROTOCOL=nmea
```

You can also configure these parameters at runtime via environment variables:
```
export WPS_SPI_GPS_DEVICE=/dev/ttyUSB0
export WPS_SPI_GPS_PROTOCOL=nmea
sudo ./skyhooklitetest
```

Finally you can exclude NMEA or SiRF code from build (to reduce binary size) via `WPS_SPI_GPS_PROTOCOL_NMEA` and `WPS_SPI_GPS_PROTOCOL_SIRF` flags (defined in `src/spi/gps/protocol/CMakeLists.txt`).

For example, if your device will always use the NMEA protocol, you can exclude the SiRF implementation by using the following parameter:
```
-DWPS_SPI_GPS_PROTOCOL_SIRF=OFF
```

### Adding your own SPI implementation

Based on the build configuration guide above, you can now predict steps for adding your own SPI implementation. For example, in order to add a new XML implementation based on `tinyxml`, you need to do the following:
* Add a subdirectory: `src/spi/xml/tinyxml`
* Write `TinyXmlParser.cpp` implementing `WPS::SPI::XmlParser` and `WPS::SPI::DOM`
* Add a `CMakeLists.txt` file to the `tinyxml` directory that creates a `libwpsspi-xml` static library target
* Run cmake with: `-DWPS_SPI_XML=tinyxml`

### Porting to a different OS

Finally, you might want to go further and port the Lite Client project to a non-Linux OS. You are free to do so given that your environment meets basic requirements (see [Extending to a non-Linux based platform](#extending-to-a-non-linux-based-platform)).

As long as those requirements have been met, porting the Lite Cient to your OS is a matter of adding corresponding SPI implementations (from a small subset up to replacing the whole SPI layer) and adding appropriate compiler specific definitions (similar to `build/unix.cmake`) to the set of `CMakeLists.txt` and `src/spi/spi.cmake`. You don't have to worry about porting the API layer code.

#### UNIX-like OS

As you may have noticed, many of the SPI modules (like `stdlibc`, `time`, `gps`) have an implementation with name `unix`. This means you can reuse those on other UNIX-like operating systems.

Other implementations you could reuse are based on open source libraries or POSIX APIs:
* `xhr` based on `libcurl`
* `xml` based on `libxml2`
* `concurrent` based on `pthreads`

For example, adding an implementation for `wifi` will be sufficient for Mac OS X support (keep a `null` cell implementation if cell support is not required).

#### Non-UNIX OS

If you want to port to something outside the UNIX world, you'll have to take into account that the C/C++ standard libraries differ across operating systems and compilers. That means you'll need to implement most of the SPI modules, including essentials like `assert`, `stdlibc`, `time` etc.

#### CMake Unavailable

This project uses CMake because it suits perfectly for a cross-platform C++ project. Its syntax is very simple and human readable. CMake is highly portable and binary distributions are available for the vast majority of platforms. It is also customizable for various cross-compiling scenarios such as using a rootstrap.

However, if your custom environment is not supported by CMake, you'll have to manually create your own `Makefile` or other kind of build script for your build system. Check out the [Build Tree](#build-tree) section to understand the build system and examine each `CMakeLists.txt` in the tree for the list of source files and macro definitions, then add them manually to your build system.

#### GPS Integration

The lite client is designed to collect sensor data from Wi-Fi, GPS and cell and provide an accurate hybrid location based on all available sensor data. The Lite Client provides a framework to allow developers to collect and submit the required sensor data and submit to Skyhook's servers for location determination.

In the `SHLC_location` method, we have implemented a simple algorithm to collect available Cell and GPS data for the duration that a Wi-Fi scan is in progress. The length of a Wi-Fi scan will generally range between 250 milliseconds to 3.5 seconds, depending on the hardware and channels scanned. Acquiring a GPS fix on the other hand can take between 30 seconds to 3 minutes (and possibly longer), depending on the hardware, available almanac and ephemeris data. For this reason, we recommend that GPS is enabled external to the `SHLC_location` method to allow the appropriate time to acquire a fix.
