cmake_minimum_required(VERSION 2.6)
project(wpsspi-xhr)

set(LITE_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/../../..)
include(${LITE_ROOT}/src/spi/spi.cmake)

check_alternate_spi_target(xhr)

if (TARGET wpsspi-xhr)
    return()
endif()

set(WPS_NO_SSL_CHECK OFF CACHE BOOL "")
mark_as_advanced(WPS_NO_SSL_CHECK)

if (WPS_NO_SSL_CHECK)
    add_definitions(-DWPS_NO_SSL_CHECK)
endif()

if (WPS_SPI_XML_HTTP_REQUEST STREQUAL "none")
    return()
elseif (UNIX)
    set(WPS_SPI_XML_HTTP_REQUEST "curl" CACHE STRING "")
elseif (NOT WPS_SPI_XML_HTTP_REQUEST)
    message(FATAL_ERROR "Unsupported platform")
endif()

add_subdirectory(${WPS_SPI_XML_HTTP_REQUEST})
mark_as_advanced(WPS_SPI_XML_HTTP_REQUEST)
