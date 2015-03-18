add_subdirectory(${LITE_SPI_ROOT}/logger logger)
add_subdirectory(${LITE_SPI_ROOT}/stdlibc stdlibc)

# For testing with phonesim
set(WPS_OFONO_DEFAULT_TO_GSM OFF CACHE BOOL "")
mark_as_advanced(WPS_OFONO_DEFAULT_TO_GSM)

if (WPS_OFONO_DEFAULT_TO_GSM)
    add_definitions(-DWPS_OFONO_DEFAULT_TO_GSM)
endif()

find_package(PkgConfig)

pkg_check_modules(GLIB2 REQUIRED glib-2.0)
pkg_check_modules(DBUS_GLIB REQUIRED dbus-glib-1)

set(MARSHALLING_TXT ${CMAKE_CURRENT_BINARY_DIR}/ofono_dbus_marshalling.txt)
set(MARSHALLING_H   ${CMAKE_CURRENT_BINARY_DIR}/ofono_dbus_marshalling.h)
set(MARSHALLING_C   ${CMAKE_CURRENT_BINARY_DIR}/ofono_dbus_marshalling.c)

file(WRITE ${MARSHALLING_TXT} "VOID:STRING,VARIANT\n")

add_custom_command(OUTPUT ${MARSHALLING_H}
                   COMMAND "glib-genmarshal"
                           "${MARSHALLING_TXT}"
                           "--header" ">" "${MARSHALLING_H}"
                   DEPENDS "${MARSHALLING_TXT}")

add_custom_command(OUTPUT ${MARSHALLING_C}
                   COMMAND "glib-genmarshal"
                           "${MARSHALLING_TXT}"
                           "--body" ">" "${MARSHALLING_C}"
                   DEPENDS "${MARSHALLING_TXT}")

include_directories(${DBUS_GLIB_INCLUDE_DIRS}
                    ${CMAKE_CURRENT_BINARY_DIR})

add_library(wpsspi-cell STATIC OfonoCellAdapter.cpp
                               ${MARSHALLING_H}
                               ${MARSHALLING_C})

target_link_libraries(wpsspi-cell wpsspi-logger
                                  wpsspi-stdlibc
                                  ${DBUS_GLIB_LIBRARIES})
