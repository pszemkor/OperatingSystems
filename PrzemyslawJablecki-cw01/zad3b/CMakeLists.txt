cmake_minimum_required(VERSION 2.8.12.2)
project(lab1)

set(CMAKE_C_STANDARD 99)

option(dyn "static")

if(${dyn} STREQUAL "dynamic")
    add_library(myLib SHARED cw1.c)
    add_executable(lab1 main.c )
    #TARGET_LINK_LIBRARIES(lab1 myLib)
    TARGET_LINK_LIBRARIES(lab1 dl) 	
    set(CMAKE_BUILD_TYPE Release)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -ldl -Wl,-rpath=./ -D DLL")
elseif(${dyn} STREQUAL "shared")
    add_library(myLib SHARED cw1.c)
    add_executable(lab1 main.c )
    TARGET_LINK_LIBRARIES(lab1 myLib)
    TARGET_LINK_LIBRARIES(lab1 dl)

    set(CMAKE_BUILD_TYPE Release)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall")
else()
    add_library(myLib STATIC cw1.c)
    add_executable(lab1 main.c )
    TARGET_LINK_LIBRARIES(lab1 myLib)
    TARGET_LINK_LIBRARIES(lab1 dl)

    set(CMAKE_BUILD_TYPE Release)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall")

endif()

option(opt "O2")

if(${opt} STREQUAL "O2")
    set(CMAKE_C_FLAGS_RELEASE "-O2")
elseif(${opt} STREQUAL "Os")
    set(CMAKE_C_FLAGS_RELEASE "-Os")
else()
    set(CMAKE_C_FLAGS_RELEASE "-O0")
endif()


