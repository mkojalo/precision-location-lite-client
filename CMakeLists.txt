cmake_minimum_required(VERSION 2.6)
project(sdk-lite)

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR} CACHE PATH "")
set(LIBRARY_OUTPUT_PATH ${EXECUTABLE_OUTPUT_PATH} CACHE PATH "")

add_subdirectory(src/api)
add_subdirectory(samples)

mark_as_advanced(CMAKE_BACKWARDS_COMPATIBILITY
                 CMAKE_INSTALL_PREFIX
                 CMAKE_BUILD_TYPE
                 EXECUTABLE_OUTPUT_PATH
                 LIBRARY_OUTPUT_PATH)
