set(WPS_ROOT_PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)
include_directories(${WPS_ROOT_PUBLIC}/include/api)

add_definitions(-DHAVE_GETOPT_H=1)

add_executable(skyhooklitetest skyhooklitetest.cpp)
target_link_libraries(skyhooklitetest skyhookliteclient)

set(SKYHOOK_API_KEY "" CACHE STRING "")

if (NOT SKYHOOK_API_KEY)
    message(FATAL_ERROR "SKYHOOK_API_KEY is undefined!")
endif()

add_definitions(-DSKYHOOK_API_KEY="${SKYHOOK_API_KEY}")

if (CMAKE_SYSTEM_NAME STREQUAL "Linux")
    set_target_properties(skyhooklitetest PROPERTIES BUILD_WITH_INSTALL_RPATH 1
                                                     INSTALL_RPATH "$ORIGIN")
endif()
