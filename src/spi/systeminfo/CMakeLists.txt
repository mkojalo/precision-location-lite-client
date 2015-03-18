cmake_minimum_required(VERSION 2.6)
project(wpsspi-systeminfo)

set(LITE_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/../../..)
include(${LITE_ROOT}/src/spi/spi.cmake)

check_alternate_spi_target(systeminfo)

if (TARGET wpsspi-systeminfo)
    return()
endif()

if (WPS_SPI_SYSTEM_INFORMATION STREQUAL "none")
    return()
elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    set(WPS_SPI_SYSTEM_INFORMATION "linux" CACHE STRING "")
else()
    set(WPS_SPI_SYSTEM_INFORMATION "null" CACHE STRING "")
endif()

add_subdirectory(${WPS_SPI_SYSTEM_INFORMATION})
mark_as_advanced(WPS_SPI_SYSTEM_INFORMATION)

