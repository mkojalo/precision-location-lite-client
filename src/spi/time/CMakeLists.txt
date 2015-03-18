cmake_minimum_required(VERSION 2.6)
project(wpsspi-time)

set(LITE_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/../../..)
include(${LITE_ROOT}/src/spi/spi.cmake)

check_alternate_spi_target(time)

if (TARGET wpsspi-time)
    return()
endif()

if (WPS_SPI_TIME STREQUAL "none")
    return()
elseif (UNIX)
    set(WPS_SPI_TIME "unix" CACHE STRING "")
elseif (NOT WPS_SPI_TIME)
    message(FATAL_ERROR "Unsupported platform")
endif()

add_subdirectory(${WPS_SPI_TIME})
mark_as_advanced(WPS_SPI_TIME)
