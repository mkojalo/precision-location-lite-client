add_subdirectory(${LITE_SPI_ROOT}/logger logger)
add_subdirectory(${LITE_SPI_ROOT}/concurrent concurrent)

add_library(wpsspi-gps-serial STATIC UnixSerialPort.cpp)
target_link_libraries(wpsspi-gps-serial wpsspi-logger wpsspi-concurrent)
