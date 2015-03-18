set(WPS_MAX_LOG_LEVEL "debug" CACHE STRING "")
mark_as_advanced(WPS_MAX_LOG_LEVEL)

if (WPS_MAX_LOG_LEVEL STREQUAL "fatal")
    add_definitions(-DWPS_MAX_LOG_LEVEL=0)
elseif (WPS_MAX_LOG_LEVEL STREQUAL "alert")
    add_definitions(-DWPS_MAX_LOG_LEVEL=1)
elseif (WPS_MAX_LOG_LEVEL STREQUAL "critical")
    add_definitions(-DWPS_MAX_LOG_LEVEL=2)
elseif (WPS_MAX_LOG_LEVEL STREQUAL "error")
    add_definitions(-DWPS_MAX_LOG_LEVEL=3)
elseif (WPS_MAX_LOG_LEVEL STREQUAL "warn")
    add_definitions(-DWPS_MAX_LOG_LEVEL=4)
elseif (WPS_MAX_LOG_LEVEL STREQUAL "notice")
    add_definitions(-DWPS_MAX_LOG_LEVEL=5)
elseif (WPS_MAX_LOG_LEVEL STREQUAL "info")
    add_definitions(-DWPS_MAX_LOG_LEVEL=6)
else()
    add_definitions(-DWPS_MAX_LOG_LEVEL=7)
endif()

add_subdirectory(${LITE_SPI_ROOT}/time time)
add_subdirectory(${LITE_SPI_ROOT}/thread thread)
add_subdirectory(${LITE_SPI_ROOT}/stdlibc stdlibc)

add_library(wpsspi-logger STATIC ${LITE_SPI_ROOT}/logger/Logger.cpp
                                 StdoutLogger.cpp)

target_link_libraries(wpsspi-logger wpsspi-time
                                    wpsspi-thread
                                    wpsspi-stdlibc)
