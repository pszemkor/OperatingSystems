cmake_minimum_required(VERSION 3.13)
project(threads_ascii_convolution C)

set(CMAKE_C_STANDARD 99)

add_executable(threads_ascii_convolution main.c im.c im.h)
target_link_libraries(threads_ascii_convolution pthread)
target_link_libraries(threads_ascii_convolution m)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall")