cmake_minimum_required(VERSION 3.0)

project(test)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS}  -g ")

add_executable(
    test
    
    test.c
)

target_link_libraries(
    test
    dl
    SDL2
)