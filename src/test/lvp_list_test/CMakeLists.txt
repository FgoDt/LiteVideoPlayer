cmake_minimum_required(VERSION 3.0)

project(list_test)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS}  -g  -Werror -Wall")

include_directories("../../../deps/ffmpeg/linux/include/")
link_directories("../../../deps/ffmpeg/linux/lib/")

add_executable(
    list_test
    ../../core/lvp_mem.h
    ../../core/lvp_list.c
    ../../core/lvp_str.c
    lvp_list_test.c
)

target_link_libraries(
    list_test
    avutil
)