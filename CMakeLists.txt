cmake_minimum_required(VERSION 3.0)


project(lvp)


include_directories("./deps/ffmpeg/linux/include/")
link_directories("./deps/ffmpeg/linux/lib/")

include_directories("./deps/ffmpeg/windows/include/")
link_directories("./deps/ffmpeg/windows/lib/")
include_directories("./deps/SDL2")
link_directories("./deps/SDL2/lib/x64/")

include_directories("./deps/soundtouch-2.3.1/include")
link_directories("./deps/soundtouch-2.3.1/lib")

include_directories("/usr/local/opt/openssl/include")
link_directories("/usr/local/opt/openssl/lib")

#if(CAMKE_HOST_APPLE)
#find_library(SDL2 SDL2)
#find_library(AVFoundation AVFoundation)
#find_library(VideoToolbox VideoToolbox)
#find_library(AudioToolbox AudioToolbox)
#find_library(CoreFoundation CoreFoundation)
#find_library(CoreMedia CoreMedia)
#find_library(CoreVideo CoreVideo)
#find_library(Security Security)
#endif()

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
 "src/core/lvp_pkt.h" "src/core/lvp_pkt.c")

add_executable( LiteVideoPlayer
        ./src/core/lvp.c
 "src/core/lvp_pkt.h" "src/core/lvp_pkt.c")

target_link_libraries(
	lvp

	avformat
    avcodec
    swresample
    swscale
    avutil
    SDL2
    SDL2main
    SoundTouchD_x64
  #  if(CAMKE_HOST_APPLE)
  #  ${SDL2}
  #  ${AVFoundation}
  #  ${VideoToolbox}
  #  ${AudioToolbox}
  #  ${CoreFoundation}
  #  ${CoreMedia}
  #  ${CoreVideo}
  #  ${Security}
  #  bz2
  #  iconv
  #  crypto
  #  ssl
  #  z
  #  m
  #  pthread
  #  lzma
  #  endif()
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
    SoundTouchD_x64
   # if(CAMKE_HOST_APPLE)
   # ${SDL2}
   # ${AVFoundation}
   # ${VideoToolbox}
   # ${AudioToolbox}
   # ${CoreFoundation}
   # ${CoreMedia}
   # ${CoreVideo}
   # ${Security}
   # bz2
   # iconv
   # crypto
   # ssl
   # z
   # m
   # pthread
   # lzma
   # endif()
)
