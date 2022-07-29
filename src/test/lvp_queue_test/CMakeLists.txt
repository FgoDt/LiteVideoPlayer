cmake_minimum_required(VERSION 3.0)

project(test)

#set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS}  -g -Werror   -Wall ")





include_directories("../../../deps/ffmpeg/windows/include/")
link_directories("../../../deps/ffmpeg/windows/lib/")

include_directories("../../../deps/SDL2/windows/")
link_directories("../../../deps/SDL2/windows/lib/x64/")

add_executable(
    test
    ../../core/lvp_nqueue.c
    lvp_queuetest.c
)

target_link_libraries(
    test
	avutil
)