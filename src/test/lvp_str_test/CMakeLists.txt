cmake_minimum_required(VERSION 3.0)

project(str_test)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS}  -g  -Werror -Wall")

include_directories("../../../deps/ffmpeg/linux/include/")
link_directories("../../../deps/ffmpeg/linux/lib/")

add_executable(
    src_test

    ../../core/lvp_str.c
    ../../core/lvp_map.c
    str_test.c
)

target_link_libraries(
    src_test
    avutil
)