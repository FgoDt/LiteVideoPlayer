#ifndef _LVP_FRAME_FILTER_H_
#define _LVP_FRAME_FILTER_H_

#include "../core/lvp_core.h"
#include <libavutil/frame.h>

typedef struct lvp_frame_filter
{
    LVPLog *log;
    LVPEventControl *ctl;
    LVPList *modules;
	AVFrame* filtered_audio_frame;
	AVFrame* filtered_video_frame;
}LVPFrameFilter;


#endif