cmake_minimum_required(VERSION 2.6)
project(wpsspi-gps-protocol)

if (TARGET wpsspi-gps-protocol)
    return()
endif()

set(WPS_SPI_GPS_PROTOCOL_NMEA ON CACHE BOOL "")
set(WPS_SPI_GPS_PROTOCOL_SIRF ON CACHE BOOL "")

mark_as_advanced(WPS_SPI_GPS_PROTOCOL_NMEA
                 WPS_SPI_GPS_PROTOCOL_SIRF)
                 
set(PROTOCOL_SOURCES GPSProtocol.h
                     GPSProtocol.cpp)

if (WPS_SPI_GPS_PROTOCOL_NMEA)
    add_subdirectory(${LITE_SPI_ROOT}/utils/nmea nmea)
    add_definitions(-DGPS_PROTOCOL_NMEA)
    include_directories(${LITE_SPI_ROOT}/utils/nmea/include)
    
    list(APPEND PROTOCOL_SOURCES nmea/NMEAProtocol.h
                                 nmea/NMEAProtocol.cpp)
endif()

if (WPS_SPI_GPS_PROTOCOL_SIRF)
    add_definitions(-DGPS_PROTOCOL_SIRF)
    list(APPEND PROTOCOL_SOURCES sirf/SirfProtocol.h
                                 sirf/SirfProtocol.cpp)
endif()

add_library(wpsspi-gps-protocol STATIC ${PROTOCOL_SOURCES})

if (WPS_SPI_GPS_PROTOCOL_NMEA)
    target_link_libraries(wpsspi-gps-protocol nmea)
endif()
