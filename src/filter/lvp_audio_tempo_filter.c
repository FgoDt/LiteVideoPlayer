#include "../core/lvp_core.h"
#include "../core/lvp.h"
#include <libavutil/frame.h>

typedef struct lvp_audio_tempo_filter {
//    void *soundTouch;
	void* sound_touch;

    LVPLog *log;
    LVPEventControl *ctl;
    unsigned char *buf;
    int max_samples;
    double tempo;
}LVPAudioTempoFilter;


static int handle_frame(LVPEvent *ev, void *usrdata){
    AVFrame *frame = (AVFrame*)ev->data;
    LVPAudioTempoFilter *f = (LVPAudioTempoFilter*)usrdata;
    //video data
    if(frame->width>0){
        return LVP_OK;
    }
    //not change
    if (f->tempo == 1.0){
        return LVP_OK;
    }
    if(f->sound_touch == NULL){
		f->sound_touch = lvp_create_soundtouch();
		lvp_soundtouch_set_channels(f->sound_touch,frame->channels);
        lvp_soundtouch_set_sample_rate(f->sound_touch,frame->sample_rate);
		lvp_soundtouch_change_tempo(f->sound_touch, (int)((f->tempo-1.0)*100));
        int size = av_get_bytes_per_sample(frame->format)*frame->channels;
        //make max size big enough
        f->max_samples = frame->nb_samples *10;
        size = size*f->max_samples;
        f->buf = lvp_mem_mallocz(size);
    
    }

    int size = av_get_bytes_per_sample(frame->format)*frame->channels;

    lvp_soundtouch_send_sample(f->sound_touch,frame->data[0],frame->nb_samples);

    AVFrame *tempo_frame = av_frame_alloc();
    int read_count = 0;
    read_count = lvp_soundtouch_recive_sample(f->sound_touch,f->buf,frame->nb_samples);
    tempo_frame->nb_samples += read_count;
	int index = 0;
    while(read_count>0){
        index  = tempo_frame->nb_samples*size;
        read_count = lvp_soundtouch_recive_sample(f->sound_touch,f->buf+index,frame->nb_samples);
        tempo_frame->nb_samples += read_count;
    }
    if (tempo_frame->nb_samples==f->max_samples){
        lvp_waring(f->log,"soundtouch may loss sample",NULL);
    }

    tempo_frame->format = frame->format;
    tempo_frame->channels = frame->channels;
    tempo_frame->sample_rate = frame->sample_rate;

    int ret = av_frame_get_buffer(tempo_frame,0);
    if (ret<0&&tempo_frame->nb_samples>0){
        av_frame_free(&tempo_frame);
        lvp_error(f->log,"got frame buffer error",NULL);
    }


    if(tempo_frame->nb_samples>0){
        memcpy(tempo_frame->data[0],f->buf,tempo_frame->nb_samples*size);
    }

    tempo_frame->pts = frame->pts;
    tempo_frame->pkt_dts = frame->pkt_dts;
    if(tempo_frame->nb_samples==0){
        tempo_frame->flags |= AV_FRAME_FLAG_DISCARD;
    }

    // free src frame 
    av_frame_free(&frame);
    ev->data = tempo_frame;
    return LVP_OK;
}

static int handle_change_tempo(LVPEvent *ev, void *usrdata){
    LVPAudioTempoFilter *f = (LVPAudioTempoFilter*)usrdata;
    int newTempo = *(int*)ev->data;
    f->tempo = newTempo;
    return LVP_OK;
}

static int filter_init(LVPModule *m,
    LVPMap *options,
    LVPEventControl *ctl,
    LVPLog *log){
    assert(m);
    assert(ctl);
    assert(log);


    LVPAudioTempoFilter *f = (LVPAudioTempoFilter*)m->private_data;
    f->ctl = ctl;
    f->log = lvp_log_alloc(m->name);
    f->log->log_call = log->log_call;
    f->log->usr_data = log->usr_data;
    
    f->tempo = 1.0;

    int ret = lvp_event_control_add_listener(f->ctl,LVP_EVENT_CHANGE_TEMPO,handle_change_tempo,f);
    ret |= lvp_event_control_add_listener(f->ctl,LVP_EVENT_FILTER_GOT_FRAME,handle_frame,f);
    if (ret != LVP_OK){
        lvp_error(f->log,"add listener error ",NULL);
    }

    return ret;
}

static void filter_close(LVPModule *m){
    LVPAudioTempoFilter *f = (LVPAudioTempoFilter*)m->private_data;
    if(f->sound_touch){
        lvp_soundtouch_destroy(f->sound_touch);
    }
    if(f->log){
        lvp_log_free(f->log);
    }
    if(f->buf){
        lvp_mem_free(f->buf);
    }
    lvp_mem_free(f);
    return ;
}


LVPModule lvp_audio_tempo_filter= {
	.version = lvp_version,
	.name = "LVP_AUDIO_TEMPO_FILTER",
	.type = LVP_MODULE_FRAME_FILTER,
	.private_data_size = sizeof(LVPAudioTempoFilter),
	.private_data = NULL,
	.module_init = filter_init,
	.module_close = filter_close,
};