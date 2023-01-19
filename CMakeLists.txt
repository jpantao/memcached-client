cmake_minimum_required(VERSION 3.24)
project(memcached_client C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_EXE_LINKER_FLAGS "-lmemcached")

add_compile_options(-Wall -Wextra -Wpedantic -g)
add_executable(memcached_client client.c)
