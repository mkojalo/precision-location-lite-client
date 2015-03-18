cmake_minimum_required(VERSION 2.6)
project(wpsspi-thread)

set(LITE_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/../../..)
include(${LITE_ROOT}/src/spi/spi.cmake)

check_alternate_spi_target(thread)

if (TARGET wpsspi-thread)
    return()
endif()

if (WPS_SPI_THREAD STREQUAL "none")
    return()
elseif (UNIX)
    set(WPS_SPI_THREAD "pthread" CACHE STRING "")
elseif (NOT WPS_SPI_THREAD)
    message(FATAL_ERROR "Unsupported platform")
endif()

add_subdirectory(${WPS_SPI_THREAD})
mark_as_advanced(WPS_SPI_THREAD)
