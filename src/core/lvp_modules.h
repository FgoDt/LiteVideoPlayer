#ifndef _LVP_MODULES_H_
#define _LVP_MODULES_H_
#include "lvp_core.h"

extern LVPModule lvp_reader_module;
extern LVPModule lvp_pkt_filter;
extern LVPModule lvp_frame_filter;
extern LVPModule lvp_audio_pkt_cache_module;
extern LVPModule lvp_video_pkt_cache_module;
extern LVPModule lvp_sub_pkt_cache_module;
extern LVPModule lvp_audio_frame_cache_module;
extern LVPModule lvp_video_frame_cache_module;
extern LVPModule lvp_sub_frame_cache_module;
extern LVPModule lvp_audio_decoder;
extern LVPModule lvp_video_decoder;
extern LVPModule lvp_sub_decoder;
extern LVPModule lvp_avsync_module;
extern LVPModule lvp_audio_resample;
extern LVPModule lvp_audio_tempo_filter;
extern LVPModule lvp_profile_module;

#ifdef LVP_DESKTOP
extern LVPModule lvp_video_render;
extern LVPModule lvp_audio_render;
#endif

#ifdef LVP_ANDROID
extern LVPModule lvp_android_video_render;
extern LVPModule lvp_android_audio_render;
#endif

//all  module
static LVPModule *LVPModules[]={
    &lvp_reader_module,
    &lvp_pkt_filter,
    &lvp_frame_filter,
    &lvp_audio_pkt_cache_module,
    &lvp_video_pkt_cache_module,
	&lvp_sub_pkt_cache_module,
    &lvp_audio_frame_cache_module,
    &lvp_video_frame_cache_module,
	&lvp_sub_frame_cache_module,
    &lvp_audio_decoder,
    &lvp_video_decoder,
	&lvp_sub_decoder,
#ifdef LVP_DESKTOP
	&lvp_video_render,
	&lvp_audio_render,
#endif
#ifdef LVP_ANDROID
	&lvp_android_video_render,
	&lvp_android_audio_render,
#endif
	&lvp_avsync_module,
	&lvp_audio_resample,
    &lvp_audio_tempo_filter,
	&lvp_profile_module,
};

static LVPModule **DynamicModules = NULL;

static int DynamicModuleNum = 0;


#endif