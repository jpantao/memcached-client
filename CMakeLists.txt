cmake_minimum_required(VERSION 3.24)
project(memcached_client C)

set(CMAKE_C_STANDARD 11)

add_executable(memcached_client client.c)
