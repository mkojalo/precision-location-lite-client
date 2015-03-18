cmake_minimum_required(VERSION 2.6)
project(wpsspi-cell)

set(LITE_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/../../..)
include(${CMAKE_CURRENT_SOURCE_DIR}/../spi.cmake)

check_alternate_spi_target(cell)

if (TARGET wpsspi-cell)
    return()
endif()

if (WPS_SPI_CELL_ADAPTER STREQUAL "none")
    return()
else()
    set(WPS_SPI_CELL_ADAPTER "null" CACHE STRING "")
endif()

add_subdirectory(${WPS_SPI_CELL_ADAPTER})
mark_as_advanced(WPS_SPI_CELL_ADAPTER)
