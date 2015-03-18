add_subdirectory(${LITE_SPI_ROOT}/stdmath stdmath)
add_library(wpsspi-time STATIC UnixTime.cpp)
target_link_libraries(wpsspi-time wpsspi-stdmath)
