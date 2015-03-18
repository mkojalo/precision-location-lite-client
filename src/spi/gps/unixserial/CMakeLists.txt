set(WPS_SPI_GPS_DEVICE "" CACHE STRING "")
set(WPS_SPI_GPS_PROTOCOL "nmea" CACHE STRING "")

mark_as_advanced(WPS_SPI_GPS_DEVICE
                 WPS_SPI_GPS_PROTOCOL)

if (WPS_SPI_GPS_DEVICE)
    add_definitions(-DWPS_SPI_GPS_DEVICE=\"${WPS_SPI_GPS_DEVICE}\")
endif()

if (WPS_SPI_GPS_PROTOCOL)
    add_definitions(-DWPS_SPI_GPS_PROTOCOL=\"${WPS_SPI_GPS_PROTOCOL}\")
endif()

include_directories(${LITE_SPI_ROOT}/gps)

add_subdirectory(${LITE_SPI_ROOT}/gps/protocol protocol)
add_subdirectory(${LITE_SPI_ROOT}/gps/serial serial)

add_library(wpsspi-gps STATIC UnixSerialGPSAdapter.cpp
                              ../GPSData.cpp)

target_link_libraries(wpsspi-gps wpsspi-time
                                 wpsspi-gps-serial
                                 wpsspi-gps-protocol)
