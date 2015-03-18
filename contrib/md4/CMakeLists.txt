cmake_minimum_required(VERSION 2.6)

add_library(md4 STATIC md4c.c)

if (CMAKE_SYSTEM_NAME STREQUAL "Linux" AND (CMAKE_SYSTEM_PROCESSOR STREQUAL "x86_64"
                                                OR CMAKE_SYSTEM_PROCESSOR STREQUAL "armv7l"))
    set_target_properties(md4 PROPERTIES COMPILE_FLAGS -fPIC)
endif()
