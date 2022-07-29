cmake_minimum_required(VERSION 3.0)

project(test)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS}  -g  -Werror -Wall")

include_directories("../../../deps/ffmpeg/linux/include/")
link_directories("../../../deps/ffmpeg/linux/lib/")

add_executable(
    test
    ../../core/lvp_log.c
    ../../core/lvp_map.c
    ../../core/lvp_event.c
    ../../core/lvp_list.c
    ../../core/lvp_str.c
    event_test.c
)

target_link_libraries(
    test
    avutil
)