cmake_minimum_required(VERSION 2.8.12)
project(lab4)

set(CMAKE_C_STANDARD 99)
add_executable(lab4 main2.c)

set(CMAKE_BUILD_TYPE Release)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall")
set(CMAKE_C_FLAGS_RELEASE "-O2"
