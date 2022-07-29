cmake_minimum_required(VERSION 3.0)

project(test)

#set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS}  -g -Werror   -Wall ")





include_directories("../../../deps/ffmpeg/linux/include/")
link_directories("../../../deps/ffmpeg/linux/lib/")

include_directories("../../../deps/SDL2/windows/")
link_directories("../../../deps/SDL2/windows/lib/x64/")

add_executable(
    test
    ../../core/lvp_core.c
    ../../core/lvp_log.c
    ../../core/lvp_map.c
    ../../core/lvp_event.c
    ../../core/lvp_list.c
    ../../core/lvp_module.c
    ../../core/lvp_thread.c
    ../../core/lvp_mutex.c
    ../../core/lvp_str.c
    ../../core/lvp_nqueue.c
	../../core/lvp_avsync.c


    ../../reader/lvp_reader_module.c
    
    ../../filter/lvp_pkt_filter.c
    ../../filter/lvp_frame_filter.c
	../../filter/lvp_audio_resample.c

    ../../cache/lvp_cache_module.c
    
    ../../decoder/lvp_decoder.c
    ../../decoder/lvp_hwaccel_decoder.c
    
    ../../render/lvp_video_render.c
	../../render/lvp_audio_render.c
    
    core_test.c
)

target_link_libraries(
    test
    avformat
    avcodec
    swresample
    swscale
    avutil
	SDL2
    SDL2main
)