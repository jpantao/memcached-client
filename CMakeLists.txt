cmake_minimum_required(VERSION 3.18)
project(memcached_client C)

set(CMAKE_C_STANDARD 11)
#set(CMAKE_EXE_LINKER_FLAGS "-lmemcached")

include_directories(/usr/lib/x86_64-linux-gnu/)
add_compile_options(-Wall -Wextra -Wpedantic -g)
add_executable(memcached_client client.c)
add_executable(memcached_client_debug client_debug.c)

target_link_libraries(memcached_client memcached)
target_link_libraries(memcached_client_debug memcached)

#if(G5K)
#    set(LIBMEMCACHED /usr/lib/x86_64-linux-gnu/libmemcached.so)
#endif()
#
#list(APPEND libs ${LIBMEMCACHED})
#target_link_libraries(memcached_client ${libs})