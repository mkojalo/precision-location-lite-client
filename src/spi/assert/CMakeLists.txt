cmake_minimum_required(VERSION 2.6)
project(wpsspi-assert)

set(LITE_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/../../..)
include(${LITE_ROOT}/src/spi/spi.cmake)

check_alternate_spi_target(assert)

if (TARGET wpsspi-assert)
    return()
endif()

if (WPS_SPI_ASSERT STREQUAL "none")
    return()
elseif(UNIX)
    set(WPS_SPI_ASSERT "unix" CACHE STRING "")
else()
    set(WPS_SPI_ASSERT "null" CACHE STRING "")
endif()

add_subdirectory(${WPS_SPI_ASSERT})
mark_as_advanced(WPS_SPI_ASSERT)
