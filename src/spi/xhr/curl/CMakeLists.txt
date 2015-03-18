add_subdirectory(${LITE_SPI_ROOT}/logger logger)
add_subdirectory(${LITE_SPI_ROOT}/stdlibc stdlibc)

find_package(CURL REQUIRED)
include_directories(${CURL_INCLUDE_DIRS})
set(CURL_SSL_LIB "nss" CACHE STRING "")

if (CURL_SSL_LIB STREQUAL "openssl")
    find_library(CURL_OPENSSL_LIB ssl)
    find_library(CURL_CRYPTO_LIB crypto)

    list(APPEND CURL_LIBRARIES ${CURL_OPENSSL_LIB} ${CURL_CRYPTO_LIB})

    add_definitions(-DCURL_USE_OPENSSL)
elseif (CURL_SSL_LIB STREQUAL "gnutls")
    find_library(CURL_GNUTLS_LIB gnutls)
    find_library(CURL_GCRYPT_LIB gcrypt)

    list(APPEND CURL_LIBRARIES ${CURL_GNUTLS_LIB} ${CURL_GCRYPT_LIB})

    add_definitions(-DCURL_USE_GNUTLS)
endif()

add_library(wpsspi-xhr STATIC CurlXmlHttpRequest.cpp)

target_link_libraries(wpsspi-xhr wpsspi-logger
                                 wpsspi-stdlibc
                                 ${CURL_LIBRARIES})
