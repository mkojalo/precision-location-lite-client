cmake_minimum_required(VERSION 2.6)
project(sdk-lite)

set(LITE_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/../..)
set(LITE_API_ROOT ${LITE_ROOT}/src/api)
set(LITE_SPI_ROOT ${LITE_ROOT}/src/spi)

function(add_spi_dependency TARGET_NAME DEPENDENCY_NAME)
    if (TARGET ${DEPENDENCY_NAME})
        target_link_libraries(${TARGET_NAME} ${DEPENDENCY_NAME})
    endif()
endfunction()

function(add_spi_dependencies TARGET_NAME)
    foreach(DEPENDENCY_NAME ${ARGN})
        add_spi_dependency(${TARGET_NAME} ${DEPENDENCY_NAME})
    endforeach()
endfunction()

add_subdirectory(${LITE_ROOT}/src/spi wpsspi)

include_directories(.
                    ${LITE_ROOT}
                    ${LITE_ROOT}/include
                    ${LITE_SPI_ROOT}/utils/xml)

if (UNIX)
    include(${LITE_ROOT}/build/unix.cmake)
endif()

include(CheckIncludeFiles)
check_include_files(endian.h HAVE_ENDIAN_H)
check_include_files(machine/endian.h HAVE_MACHINE_ENDIAN_H)

if (HAVE_ENDIAN_H)
    add_definitions(-DHAVE_ENDIAN_H)
endif()

if (HAVE_MACHINE_ENDIAN_H)
    add_definitions(-DHAVE_MACHINE_ENDIAN_H)
endif()

include_directories(${LITE_ROOT}/contrib/md4)
add_subdirectory(${LITE_ROOT}/contrib/md4 md4)

add_library(skyhookliteclient SHARED ${LITE_API_ROOT}/Protocol.h
                                     ${LITE_API_ROOT}/Protocol.cpp
                                     ${LITE_API_ROOT}/Wrappers.h
                                     ${LITE_API_ROOT}/Wrappers.cpp
                                     ${LITE_ROOT}/include/api/skyhookliteclient.h
                                     ${LITE_API_ROOT}/skyhookliteclient.cpp
                                     ${LITE_SPI_ROOT}/utils/xml/XmlUtils.h
                                     ${LITE_SPI_ROOT}/utils/xml/XmlUtils.cpp)

add_spi_dependencies(skyhookliteclient wpsspi-assert
                                       wpsspi-concurrent
                                       wpsspi-logger
                                       wpsspi-xml
                                       wpsspi-xhr
                                       wpsspi-stdlibc
                                       wpsspi-stdmath
                                       wpsspi-thread
                                       wpsspi-time
                                       wpsspi-wifi
                                       wpsspi-gps
                                       wpsspi-cell
                                       wpsspi-systeminfo)

target_link_libraries(skyhookliteclient md4)
