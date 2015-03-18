add_subdirectory(${LITE_SPI_ROOT}/time time)
add_subdirectory(${LITE_SPI_ROOT}/stdlibc stdlibc)
add_subdirectory(${LITE_SPI_ROOT}/stdmath stdmath)

add_library(wpsspi-wifi STATIC StaticWifiAdapter.cpp
                               ../MAC.cpp)

target_link_libraries(wpsspi-wifi wpsspi-time
                                  wpsspi-stdlibc
                                  wpsspi-stdmath)
