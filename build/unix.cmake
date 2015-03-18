if (CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fvisibility=hidden -fno-rtti")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fvisibility=hidden")
endif()

if (CYGWIN)
    return()
endif()

add_definitions(-DHAVE_XLOCALE)

if (CMAKE_SYSTEM_NAME STREQUAL "Linux")
    add_definitions(-DHAVE_STD_LOCALE)

    if (CMAKE_SYSTEM_PROCESSOR STREQUAL "x86_64" OR CMAKE_SYSTEM_PROCESSOR STREQUAL "armv7l")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC")
    endif()
elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    add_definitions(-DHAVE_PRINTF_L)

    if (NOT CMAKE_OSX_ARCHITECTURES)
        set(CMAKE_OSX_ARCHITECTURES "i386;x86_64")
    endif()
endif()
