cmake_minimum_required(VERSION 2.6)
project(wpsspi-wifi)

set(LITE_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/../../..)
include(${LITE_ROOT}/src/spi/spi.cmake)

check_alternate_spi_target(wifi)

if (TARGET wpsspi-wifi)
    return()
endif()

if (WPS_SPI_WIFI_ADAPTER STREQUAL "none")
    return()
elseif (CMAKE_SYSTEM_NAME STREQUAL "Linux")
    set(WPS_SPI_WIFI_ADAPTER "nl80211" CACHE STRING "")
else()
    set(WPS_SPI_WIFI_ADAPTER "static" CACHE STRING "")
endif()

add_subdirectory("${WPS_SPI_WIFI_ADAPTER}")
mark_as_advanced(WPS_SPI_WIFI_ADAPTER)
