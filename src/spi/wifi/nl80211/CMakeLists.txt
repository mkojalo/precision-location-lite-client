add_subdirectory(${LITE_SPI_ROOT}/logger logger)
add_subdirectory(${LITE_SPI_ROOT}/concurrent concurrent)
add_subdirectory(${LITE_SPI_ROOT}/time time)

find_package(PkgConfig)
pkg_check_modules(NL REQUIRED libnl-3.0)
pkg_check_modules(NL_GENL REQUIRED libnl-genl-3.0)
pkg_check_modules(NL_ROUTE REQUIRED libnl-route-3.0)

include_directories(${NL_INCLUDE_DIRS}
                    ${NL_GENL_INCLUDE_DIRS}
                    ${NL_ROUTE_INCLUDE_DIRS})

add_library(wpsspi-wifi STATIC Nl80211WifiAdapter.cpp
                               ${LITE_SPI_ROOT}/wifi/MAC.cpp)

target_link_libraries(wpsspi-wifi wpsspi-logger
                                  wpsspi-time
                                  ${NL_LIBRARIES}
                                  ${NL_GENL_LIBRARIES}
                                  ${NL_ROUTE_LIBRARIES})
