cmake_minimum_required(VERSION 2.6)
project(wpsspi-stdlibc)

set(LITE_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/../../..)
include(${LITE_ROOT}/src/spi/spi.cmake)

check_alternate_spi_target(stdlibc)

if (TARGET wpsspi-stdlibc)
    return()
endif()

if (WPS_SPI_STDLIBC STREQUAL "none")
    return()
elseif (UNIX)
    set(WPS_SPI_STDLIBC "unix" CACHE STRING "")
elseif (NOT WPS_SPI_STDLIBC)
    message(FATAL_ERROR "Unsupported platform")
endif()

add_subdirectory(${WPS_SPI_STDLIBC})
mark_as_advanced(WPS_SPI_STDLIBC)
