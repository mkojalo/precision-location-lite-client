add_subdirectory(${LITE_SPI_ROOT}/logger logger)
add_library(wpsspi-systeminfo STATIC LinuxSystemInformation.cpp)
target_link_libraries(wpsspi-systeminfo wpsspi-logger)
