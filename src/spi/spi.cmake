if (NOT LITE_ROOT)
    message(FATAL_ERROR "LITE_ROOT must be defined")
endif()

set(LITE_SPI_ROOT ${LITE_ROOT}/src/spi)
set(LITE_SPI_UTILS ${LITE_SPI_ROOT}/utils)

include_directories(${LITE_ROOT}/include
                    ${LITE_SPI_UTILS})

if (UNIX)
    include_directories(${LITE_SPI_UTILS}/unix)
    include(${LITE_ROOT}/build/unix.cmake)
endif()

function(check_alternate_spi_target TARGET_SUBDIR)
    if (NOT ALTERNATE_SPI_ROOT)
        return()
    endif()

    set(ALTERNATE_TARGET_PATH "${ALTERNATE_SPI_ROOT}/${TARGET_SUBDIR}")
    if (EXISTS ${ALTERNATE_TARGET_PATH})
        add_subdirectory(${ALTERNATE_TARGET_PATH} ${TARGET_SUBDIR})
    endif()
endfunction()
