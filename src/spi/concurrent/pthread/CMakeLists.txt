find_library(PTHREAD_LIBRARY pthread)
add_library(wpsspi-concurrent STATIC PthreadConcurrent.cpp)
target_link_libraries(wpsspi-concurrent ${PTHREAD_LIBRARY})
