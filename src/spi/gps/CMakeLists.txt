cmake_minimum_required(VERSION 2.6)
project(wpsspi-gps)

set(LITE_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/../../..)
include(${CMAKE_CURRENT_SOURCE_DIR}/../spi.cmake)

check_alternate_spi_target(gps)

if (TARGET wpsspi-gps)
    return()
endif()

if (WPS_SPI_GPS_ADAPTER STREQUAL "none")
    return()
else()
    set(WPS_SPI_GPS_ADAPTER "null" CACHE STRING "")
endif()

add_subdirectory(${WPS_SPI_GPS_ADAPTER})
mark_as_advanced(WPS_SPI_GPS_ADAPTER)
