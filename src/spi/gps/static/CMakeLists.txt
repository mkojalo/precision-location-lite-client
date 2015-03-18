add_subdirectory(${LITE_SPI_ROOT}/time time)
add_library(wpsspi-gps STATIC StaticGPSAdapter.cpp ../GPSData.cpp)
target_link_libraries(wpsspi-gps wpsspi-time)
