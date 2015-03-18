cmake_minimum_required(VERSION 2.6)
project(wpsspi-xml)

set(LITE_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/../../..)
include(${LITE_ROOT}/src/spi/spi.cmake)

check_alternate_spi_target(xml)

if (TARGET wpsspi-xml)
    return()
endif()

if (WPS_SPI_XML STREQUAL "none")
    return()
elseif (UNIX)
    set(WPS_SPI_XML "libxml" CACHE STRING "")
elseif (NOT WPS_SPI_XML)
    message(FATAL_ERROR "Unsupported platform")
endif()

add_subdirectory(${WPS_SPI_XML})
mark_as_advanced(WPS_SPI_XML)
