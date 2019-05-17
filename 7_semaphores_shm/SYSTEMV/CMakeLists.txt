project(zad7)

cmake_minimum_required(VERSION 2.8)

add_library(common common.c)

add_executable(loader loader.c)
add_executable(trucker truck.c)
add_executable(init init.c)

target_link_libraries(loader common)
target_link_libraries(trucker common)
target_link_libraries(init common)
set(CMAKE_BUILD_TYPE Release)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall")
set(CMAKE_C_FLAGS_RELEASE "-O2")
