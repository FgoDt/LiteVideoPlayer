#ifndef _LVP_VIDEO_RENDER_H_
#define _LVP_VIDEO_RENDER_H_

#include "../core/lvp_core.h"
#include <SDL2/SDL.h>

typedef struct lvp_video_render {
    LVPEventControl *ctl;
    LVPLog *log;

    SDL_Window *window;
    SDL_Texture *texture;
    SDL_Renderer *renderer;
    SDL_Rect rect;
	lvp_mutex mutex;

    int cur_width;
    int cur_height;
    int64_t seek_time;
}LVPVideoRender;

#endif // !_LVP_VIDEO_RENDER_H_
