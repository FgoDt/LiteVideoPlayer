#include "lvp_audio_render.h"
#include <libavutil/time.h>
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>

static void audio_call(void *usrdata,uint8_t *stream, int len){
    LVPAudioRender *r = (LVPAudioRender*)usrdata;
    uint8_t *mixdata = (uint8_t*)malloc(len);
    memset(mixdata,0,len);
    memset(stream,0,len);
	lvp_mutex_lock(&r->buf_mutex);
    if(r->buf_len >= len){

        int can_write = r->buf_max - r->rpos;
        can_write = can_write > len ? len: can_write;
        memcpy(mixdata,r->buf+r->rpos,can_write);

        if(can_write<len){
            r->rpos = 0;
            int less = len -can_write;
            memcpy(mixdata+can_write,r->buf+r->rpos,less);
            r->rpos += less;
        }else
        {
            r->rpos+=can_write;
        }
		r->buf_len -= len;
		r->buf_time = r->buf_len * 1000 / r->audio_spec->freq / r->one_sample_size;
    }
    else{
        printf("LAG\n");
    }
	lvp_mutex_unlock(&r->buf_mutex);
	

    SDL_MixAudioFormat(stream,mixdata,r->audio_spec->format,len,SDL_MIX_MAXVOLUME);
    free(mixdata);
}

static int init_sdl_audio(LVPAudioRender *r,AVFrame *f){
    SDL_AudioSpec wanted;
    wanted.channels = f->channels;
    switch (f->format)
    {
    case AV_SAMPLE_FMT_U8:
        wanted.format = AUDIO_U8;
        break;
    case AV_SAMPLE_FMT_S16:
        wanted.format = AUDIO_S16;
        break;
    case AV_SAMPLE_FMT_S32:
        wanted.format = AUDIO_S32;
        break;
    case AV_SAMPLE_FMT_FLT:
        wanted.format = AUDIO_F32;
        break;
    default:
        lvp_error(r->log,"format %d is not support for sdl audio render",f->format);
        return LVP_E_FATAL_ERROR;
        break;
    }

	r->audio_spec = lvp_mem_mallocz(sizeof(SDL_AudioSpec));
    wanted.silence = 0;
    wanted.samples = 1024;
    wanted.userdata = r;
    wanted.callback = audio_call;
	wanted.freq = f->sample_rate;
	SDL_Init(SDL_INIT_AUDIO);
	r->audio_deviece = SDL_OpenAudioDevice(NULL, 0, &wanted, r->audio_spec, 0);
	//SDL_OpenAudio(&wanted, r->audio_spec);
	r->one_sample_size = av_get_bytes_per_sample(f->format) * f->channels;
    if(r->audio_deviece <2 ){
        lvp_error(r->log,"open audio device error",NULL);
        return LVP_E_FATAL_ERROR;
    }
	SDL_PauseAudioDevice(r->audio_deviece, 0);
	//SDL_PauseAudio(0);
    return LVP_OK;
}

static int handle_audio(LVPEvent *ev, void *usrdata){

    AVFrame *frame = (AVFrame*)ev->data;
    LVPAudioRender *r = (LVPAudioRender*)usrdata;

	size_t one_frame_size = av_get_bytes_per_sample(frame->format);
	size_t size = one_frame_size * frame->nb_samples * frame->channels;
	if(r->buf == NULL){
        size_t min_size = one_frame_size * 1024 * frame->channels * 6;
        r->buf_max = size * 6;
        r->buf_max = r->buf_max < min_size ? min_size : r->buf_max;
        r->buf =(uint8_t*)lvp_mem_mallocz(r->buf_max);
        int ret = init_sdl_audio(r,frame);
        SDL_Init(SDL_INIT_AUDIO);
        if(ret!=LVP_OK){
            lvp_error(r->log,"init audio error",NULL);
            return LVP_E_FATAL_ERROR;
        }
    }
    int inbuf_size = size;
    if(inbuf_size+r->buf_len > r->buf_max){
        //tell avsync audio buf time;
		r->buf_time = r->buf_len * 1000 / r->audio_spec->freq / r->one_sample_size;
        ev->data = &r->buf_time;
        return LVP_E_NO_MEM;
    }

	lvp_mutex_lock(&r->buf_mutex);
    uint64_t  can_write = r->buf_max - r->wpos;
    can_write = can_write > inbuf_size ? inbuf_size :can_write;
    memcpy(r->buf+r->wpos,frame->data[0],can_write);
    if(can_write<inbuf_size){
        int less = inbuf_size - can_write;
        r->wpos = 0;
        memcpy(r->buf+r->wpos,frame->data[0]+can_write,less);
        r->wpos+=less;
    }else
    {
		r->wpos += inbuf_size;
    }
    r->buf_len += inbuf_size;

    ev->data = &r->buf_time;
	lvp_mutex_unlock(&r->buf_mutex);
    return LVP_OK;

}

static int module_init(struct lvp_module *module, 
                                    LVPMap *options,
                                    LVPEventControl *ctl,
                                    LVPLog *log){
    assert(module);
    assert(ctl);
    assert(log);


    LVPAudioRender *r = (LVPAudioRender*)module->private_data;
    r->ctl = ctl;
    r->log = lvp_log_alloc(module->name);
    r->log->log_call = log->log_call;
    r->log->usr_data = log->usr_data;
	
	lvp_mutex_create(&r->buf_mutex);

    int ret = lvp_event_control_add_listener(ctl,LVP_EVENT_UPDATE_AUDIO,handle_audio,r);

    if(ret != LVP_OK){
        lvp_error(r->log,"add listener error",NULL);
        return LVP_E_FATAL_ERROR;
    }


    return LVP_OK;
}


static void module_close(struct lvp_module *module){
    assert(module);

	LVPAudioRender* r = (LVPAudioRender*)module->private_data;
	if (r->audio_deviece >= 2) {
		SDL_PauseAudioDevice(r->audio_deviece, 1);
		SDL_CloseAudioDevice(r->audio_deviece);
	}
	if (r->audio_spec) {
		lvp_mem_free(r->audio_spec);
	}
	if (r->buf) {
		lvp_mem_free(r->buf);
	}
	if (r->log) {
		lvp_log_free(r->log);
	}
	lvp_mem_free(r);
	module->private_data = NULL;

}

LVPModule lvp_audio_render = {
    .version = lvp_version,
    .name = "LVP_AUDIO_RENDER",
    .type = LVP_MODULE_CORE|LVP_MODULE_RENDER,
    .private_data_size = sizeof(LVPAudioRender),
    .private_data = NULL,
    .module_init = module_init,
    .module_close = module_close,
};