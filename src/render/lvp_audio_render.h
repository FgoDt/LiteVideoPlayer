#ifndef _LVP_AUDIO_RENDER_H_
#define _LVP_AUDIO_RENDER_H_
#include "../core/lvp_core.h"
#include <SDL2/SDL.h>

typedef struct lvp_audio_render {
    LVPEventControl *ctl;
    LVPLog *log;
	uint64_t buf_time;

    SDL_AudioSpec *audio_spec;
    SDL_AudioDeviceID audio_deviece;
	int one_sample_size;
    uint8_t *buf;
    uint64_t buf_max;
    uint64_t buf_len;
    uint64_t rpos;
    uint64_t wpos;

	lvp_mutex buf_mutex;
}LVPAudioRender;


#endif // !_LVP_RENDER_H_
