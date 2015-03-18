cmake_minimum_required(VERSION 2.6)
project(wpsspi-stdmath)

set(LITE_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/../../..)
include(${LITE_ROOT}/src/spi/spi.cmake)

check_alternate_spi_target(stdmath)

if (TARGET wpsspi-stdmath)
    return()
endif()

if (WPS_SPI_STDMATH STREQUAL "none")
    return()
elseif (UNIX)
    set(WPS_SPI_STDMATH "unix" CACHE STRING "")
elseif (NOT WPS_SPI_STDMATH)
    message(FATAL_ERROR "Unsupported platform")
endif()

add_subdirectory(${WPS_SPI_STDMATH})
mark_as_advanced(WPS_SPI_STDMATH)
