#include "../core/lvp_core.h"
#include <libavcodec/avcodec.h>

typedef struct lvp_start_on_time{
    LVPLog *log;
    LVPEventControl *ctl;
    int64_t start_time;
}LVPStartOnTime;

static int handle_on_time(LVPEvent *ev, void *usrdata){
    LVPStartOnTime *f = (LVPStartOnTime*)usrdata;
    int64_t time = *(int64_t*)ev->data;
    if (time<0){
        return LVP_E_PARAM_ERROR;
    }
    f->start_time = time;
    return LVP_OK;
}

static int handle_frame(LVPEvent *ev, void *usrdata){
    AVFrame *frame = (AVFrame*)ev->data;
    LVPStartOnTime *f = (LVPStartOnTime*)usrdata;
    //this frame need display
    if (frame->pkt_pts>=f->start_time){
        return LVP_OK;
    }
    frame->flags |= AV_FRAME_FLAG_DISCARD;
    return LVP_OK;
}

static int filter_init(LVPModule *m, LVPMap *option, LVPEventControl *ctl, LVPLog *log){
    assert(m);
    assert(ctl);
    LVPStartOnTime *f = (LVPStartOnTime*)m->private_data;
    f->ctl = ctl;
    f->log = lvp_log_alloc(m->name);
    f->log->usr_data= log->usr_data;
    f->start_time = 0;

    int ret = lvp_event_control_add_listener(ctl,LVP_EVENT_PLAY_ON,handle_on_time,f);
    if(ret!=LVP_OK){
        lvp_error(f->log,"add event %s error",LVP_EVENT_PLAY_ON);
        return ret;
    }
    ret = lvp_event_control_add_listener(ctl,LVP_EVENT_FILTER_GOT_FRAME,handle_frame,f);
    if(ret != LVP_OK){
        lvp_error(f->log,"add event %s error",LVP_EVENT_FILTER_GOT_FRAME);
        return ret;
    }

    return LVP_OK;
}

static int filter_close(LVPModule *m){
    assert(m);
    LVPStartOnTime *f = (LVPStartOnTime*)m->private_data;
}

LVPModule lvp_play_on_time = {
	.version = lvp_version,
	.name = "LVP_PLAY_ON_TIME",
	.type = LVP_MODULE_FRAME_FILTER,
	.private_data_size = sizeof(LVPStartOnTime),
	.private_data = NULL,
	.module_init = filter_init,
	.module_close = filter_close,
};