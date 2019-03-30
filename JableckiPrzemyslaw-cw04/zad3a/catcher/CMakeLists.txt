cmake_minimum_required(VERSION 2.8.12)
project(catcher)

set(CMAKE_C_STANDARD 99)
add_executable(catcher catcher.c)

set(CMAKE_BUILD_TYPE Release)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall")
set(CMAKE_C_FLAGS_RELEASE "-O2")
