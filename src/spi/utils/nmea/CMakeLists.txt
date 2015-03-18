cmake_minimum_required(VERSION 2.4)
project(wpsspi-nmea)

if (TARGET nmea)
    return()
endif()

include_directories(include)

set(LITE_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/../../../..)

include(${LITE_ROOT}/src/spi/spi.cmake)

add_subdirectory(${LITE_SPI_ROOT}/stdlibc stdlibc)
add_subdirectory(${LITE_SPI_ROOT}/stdmath stdmath)
add_subdirectory(${LITE_SPI_ROOT}/time time)
add_subdirectory(${LITE_SPI_ROOT}/assert assert)

add_library(nmea STATIC include/nmea/Dataset.h
                        include/nmea/Fields.h
                        include/nmea/Info.h
                        include/nmea/NMEA.h
                        include/nmea/Types.h
                        include/nmea/Units.h
                        include/nmea/Variant.h
                        src/GGA.h
                        src/GSA.h
                        src/GSA1.h
                        src/GSV.cpp
                        src/GSV.h
                        src/NMEA.cpp
                        src/RMC.h
                        src/RMC1.h
                        src/Sentence.cpp
                        src/Sentence.h
                        src/Token.h
                        src/Tokens.h)

target_link_libraries(nmea wpsspi-stdlibc
                           wpsspi-stdmath
                           wpsspi-time
                           wpsspi-assert)
