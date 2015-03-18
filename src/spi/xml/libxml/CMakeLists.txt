if (CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    find_library(ICONV_LIB iconv)
    find_library(LIBXML2_LIBRARIES xml2)
    find_path(LIBXML2_INCLUDE_DIR libxml/parser.h PATHS /usr/include/libxml2)
else()
    find_package(LibXml2 REQUIRED)
    if (CYGWIN)
        find_library(ICONV_LIB iconv)
    endif()
endif()

include_directories(${LIBXML2_INCLUDE_DIR})

add_subdirectory(${LITE_SPI_ROOT}/logger logger)

add_library(wpsspi-xml STATIC LibxmlDOM.cpp
                              LibxmlParser.cpp)

target_link_libraries(wpsspi-xml wpsspi-logger
                                 ${LIBXML2_LIBRARIES}
                                 ${ICONV_LIB})
