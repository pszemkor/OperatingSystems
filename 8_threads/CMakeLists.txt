cmake_minimum_required(VERSION 3.13)
project(threaaads C)

set(CMAKE_C_STANDARD 99)

add_executable(threaaads main.c)
target_link_libraries(threaaads pthread)