cmake_minimum_required(VERSION 2.6)
project(wpsspi-gps-serial)

set(LITE_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/../../../..)
include(${LITE_ROOT}/src/spi/spi.cmake)

check_alternate_spi_target(gps/serial)

if (TARGET wpsspi-gps-serial)
    return()
endif()

if (WPS_SPI_SERIAL_PORT STREQUAL "none")
    return()
elseif (UNIX)
    set(WPS_SPI_SERIAL_PORT "unix" CACHE STRING "")
elseif (NOT WPS_SPI_SERIAL_PORT)
    message(FATAL_ERROR "Unsupported platform")
endif()

add_subdirectory(${WPS_SPI_SERIAL_PORT})
mark_as_advanced(WPS_SPI_SERIAL_PORT)
