cmake_minimum_required(VERSION 3.0)

project(map_test)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS}  -g  -Werror -Wall")

include_directories("../../../deps/ffmpeg/linux/include/")
link_directories("../../../deps/ffmpeg/linux/lib/")

add_executable(
    map_test
    ../../core/lvp_map.c
    ../../core/lvp_str.c
    map_test.c
)

target_link_libraries(
    map_test
    avutil
)