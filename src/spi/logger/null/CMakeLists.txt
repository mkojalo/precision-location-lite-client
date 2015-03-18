add_subdirectory(${LITE_SPI_ROOT}/stdlibc stdlibc)
add_library(wpsspi-logger STATIC ../Logger.cpp NullLogger.cpp)
target_link_libraries(wpsspi-logger wpsspi-stdlibc)
