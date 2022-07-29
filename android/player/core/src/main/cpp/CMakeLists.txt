cmake_minimum_required(VERSION 3.10.2)

add_definitions(-DLVP_LINUX -DLVP_ANDROID)

project("lvpjni")

include_directories("../../../../../../deps/android/${CMAKE_ANDROID_ARCH_ABI}/include/")
include_directories("../../../../../../deps/android/SoundTouch/include/")
link_directories("../../../../../../deps/android/${CMAKE_ANDROID_ARCH_ABI}/lib/")

add_library(
        SoundTouch
        ../../../../../../deps/android/SoundTouch/source/SoundTouch/AAFilter.cpp
        ../../../../../../deps/android/SoundTouch/source/SoundTouch/FIFOSampleBuffer.cpp
        ../../../../../../deps/android/SoundTouch/source/SoundTouch/FIRFilter.cpp
        ../../../../../../deps/android/SoundTouch/source/SoundTouch/cpu_detect_x86.cpp
        ../../../../../../deps/android/SoundTouch/source/SoundTouch/sse_optimized.cpp
        ../../../../../../deps/android/SoundTouch/source/SoundStretch/WavFile.cpp
        ../../../../../../deps/android/SoundTouch/source/SoundTouch/RateTransposer.cpp
        ../../../../../../deps/android/SoundTouch/source/SoundTouch/SoundTouch.cpp
        ../../../../../../deps/android/SoundTouch/source/SoundTouch/InterpolateShannon.cpp
        ../../../../../../deps/android/SoundTouch/source/SoundTouch/InterpolateCubic.cpp
        ../../../../../../deps/android/SoundTouch/source/SoundTouch/InterpolateLinear.cpp
        ../../../../../../deps/android/SoundTouch/source/SoundTouch/TDStretch.cpp
        ../../../../../../deps/android/SoundTouch/source/SoundTouch/BPMDetect.cpp
        ../../../../../../deps/android/SoundTouch/source/SoundTouch/PeakFinder.cpp
)

add_library(
        lvp
        ../../../../../../src/core/lvp_array.c
        ../../../../../../src/core/lvp_avsync.c
        ../../../../../../src/core/lvp_condition.c
        ../../../../../../src/core/lvp_core.c
        ../../../../../../src/core/lvp_event.c
        ../../../../../../src/core/lvp_list.c
        ../../../../../../src/core/lvp_log.c
        ../../../../../../src/core/lvp_map.c
        ../../../../../../src/core/lvp_module.c
        ../../../../../../src/core/lvp_mutex.c
        ../../../../../../src/core/lvp_nqueue.c
        ../../../../../../src/core/lvp_queue.c
        ../../../../../../src/core/lvp_stack.c
        ../../../../../../src/core/lvp_str.c
        ../../../../../../src/core/lvp_time.c
        ../../../../../../src/core/lvp_thread.c
        ../../../../../../src/core/lvp_cpp.cc
        ../../../../../../src/core/lvp_pkt.c
        ../../../../../../src/core/lvp_profile.c

        ../../../../../../src/decoder/lvp_decoder.c
        ../../../../../../src/decoder/lvp_hwaccel_decoder.c

        ../../../../../../src/filter/lvp_audio_resample.c
        ../../../../../../src/filter/lvp_frame_filter.c
        ../../../../../../src/filter/lvp_pkt_filter.c
        ../../../../../../src/filter/lvp_audio_tempo_filter.c

        ../../../../../../src/reader/lvp_reader_module.c

        android_video_render.c
        android_audio_render.c

        ../../../../../../src/cache/lvp_cache_module.c
)

add_library( # Sets the name of the library.
        lvpjni

        # Sets the library as a shared library.
        SHARED

        # Provides a relative path to your source file(s).
        core.cpp
        )

find_library( # Sets the name of the path variable.
        log-lib

        log )

target_link_libraries( # Specifies the target library.
        lvp
        SoundTouch
        swresample
        swscale
        avutil
        avformat
        avcodec
        )

target_link_libraries( # Specifies the target library.
        lvpjni

        ${log-lib}
        lvp
        SoundTouch
        swresample
        swscale
        avformat
        avcodec
        avutil
        EGL
        GLESv3
        OpenSLES
        android
        z
        )
