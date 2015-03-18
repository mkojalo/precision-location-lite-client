cmake_minimum_required(VERSION 2.6)
project(wpsspi-logger)

set(LITE_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/../../..)
include(${LITE_ROOT}/src/spi/spi.cmake)

check_alternate_spi_target(logger)

if (TARGET wpsspi-logger)
    return()
endif()

if (WPS_SPI_LOGGER STREQUAL "none")
    return()
else()
    set(WPS_SPI_LOGGER "stdout" CACHE STRING "")
endif()

add_subdirectory(${WPS_SPI_LOGGER})
mark_as_advanced(WPS_SPI_LOGGER)
