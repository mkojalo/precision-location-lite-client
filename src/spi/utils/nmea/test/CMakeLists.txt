cmake_minimum_required(VERSION 2.6)
project(nmea-test)

set(LITE_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/../../../../..)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../include
                    ${LITE_ROOT}/include)

add_subdirectory(.. nmea)

add_executable(test-nmea test.cpp)
target_link_libraries(test-nmea nmea)
