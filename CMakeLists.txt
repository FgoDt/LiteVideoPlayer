cmake_minimum_required(VERSION 3.0)

add_definitions(-DLVP_WIN -DLVP_DESKTOP)

project(lvp)


include_directories("./deps/win64/ffmpeg/include/")
link_directories("./deps/win64/ffmpeg/lib/")
include_directories("./deps/win64")
link_directories("./deps/win64/SDL2/lib/x64/")

include_directories("./deps/android/SoundTouch/include/")

add_library(
        SoundTouch
        ./deps/android/SoundTouch/source/SoundTouch/AAFilter.cpp
        ./deps/android/SoundTouch/source/SoundTouch/FIFOSampleBuffer.cpp
        ./deps/android/SoundTouch/source/SoundTouch/FIRFilter.cpp
        ./deps/android/SoundTouch/source/SoundTouch/cpu_detect_x86.cpp
        ./deps/android/SoundTouch/source/SoundTouch/sse_optimized.cpp
        ./deps/android/SoundTouch/source/SoundStretch/WavFile.cpp
        ./deps/android/SoundTouch/source/SoundTouch/RateTransposer.cpp
        ./deps/android/SoundTouch/source/SoundTouch/SoundTouch.cpp
        ./deps/android/SoundTouch/source/SoundTouch/InterpolateShannon.cpp
        ./deps/android/SoundTouch/source/SoundTouch/InterpolateCubic.cpp
        ./deps/android/SoundTouch/source/SoundTouch/InterpolateLinear.cpp
        ./deps/android/SoundTouch/source/SoundTouch/TDStretch.cpp
        ./deps/android/SoundTouch/source/SoundTouch/BPMDetect.cpp
        ./deps/android/SoundTouch/source/SoundTouch/PeakFinder.cpp
)

add_library(lvp 
            ./src/core/lvp_array.c
            ./src/core/lvp_avsync.c
            ./src/core/lvp_condition.c
            ./src/core/lvp_core.c
            ./src/core/lvp_event.c
            ./src/core/lvp_list.c
            ./src/core/lvp_log.c
            ./src/core/lvp_map.c
            ./src/core/lvp_module.c
            ./src/core/lvp_mutex.c
            ./src/core/lvp_nqueue.c
            ./src/core/lvp_queue.c
            ./src/core/lvp_stack.c
            ./src/core/lvp_str.c
            ./src/core/lvp_time.c
            ./src/core/lvp_thread.c
            ./src/core/lvp_cpp.cc

            ./src/decoder/lvp_decoder.c
            ./src/decoder/lvp_hwaccel_decoder.c

            ./src/filter/lvp_audio_resample.c
            ./src/filter/lvp_frame_filter.c
            ./src/filter/lvp_pkt_filter.c
            ./src/filter/lvp_audio_tempo_filter.c

            ./src/reader/lvp_reader_module.c

            ./src/render/lvp_audio_render.c
            ./src/render/lvp_video_render.c

            ./src/cache/lvp_cache_module.c
            src/core/lvp_pkt.c
            src/core/lvp_profile.c)

add_executable( LiteVideoPlayer
        ./src/core/lvp.c)

target_link_libraries(
	lvp

	avformat
    avcodec
    swresample
    swscale
    avutil
    SDL2
    SDL2main
    SoundTouch
)

target_link_libraries(
    LiteVideoPlayer
    lvp

    avformat
    avcodec
    swresample
    swscale
    avutil
    SDL2
    SDL2main
    SoundTouch
)
