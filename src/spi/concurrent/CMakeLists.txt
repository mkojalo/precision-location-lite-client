cmake_minimum_required(VERSION 2.6)
project(wpsspi-concurrent)

set(LITE_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/../../..)
include(${LITE_ROOT}/src/spi/spi.cmake)

check_alternate_spi_target(concurrent)

if (TARGET wpsspi-concurrent)
    return()
endif()

if (WPS_SPI_CONCURRENT STREQUAL "none")
    return()
elseif (UNIX)
    set(WPS_SPI_CONCURRENT "pthread" CACHE STRING "")
elseif (NOT WPS_SPI_CONCURRENT)
    message(FATAL_ERROR "Unsupported platform")
endif()

add_subdirectory(${WPS_SPI_CONCURRENT})
mark_as_advanced(WPS_SPI_CONCURRENT)
