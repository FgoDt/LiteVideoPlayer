#include "lvp_frame_filter.h"
#include <libavcodec/avcodec.h>


static void filter_sub_module_free(void *data,void *usr_data){
    LVPModule *m = (LVPModule*)data;
    lvp_module_close(m);
    lvp_mem_free(m);
}

static int handle_frame(LVPEvent *ev,void *usr_data){
    assert(ev);
    LVPFrameFilter *f = (LVPFrameFilter*)usr_data;
	AVFrame* filtered_frame = NULL;
	AVFrame* src_frame = (AVFrame*)ev->data;
	enum AVMediaType frame_type = AVMEDIA_TYPE_UNKNOWN;
	if (src_frame->width > 0) {
		filtered_frame = f->filtered_video_frame;
		frame_type = AVMEDIA_TYPE_VIDEO;
	}if (src_frame->channels > 0) {
		filtered_frame = f->filtered_audio_frame;
		frame_type = AVMEDIA_TYPE_AUDIO;
	}

	//resend filted frame
	if (filtered_frame) {
		LVPEvent* must_handle_ev = lvp_event_alloc(filtered_frame, LVP_EVENT_FILTER_SEND_FRAME, LVP_TRUE);
		int ret = lvp_event_control_send_event(f->ctl, must_handle_ev);
		if (ret == LVP_E_NO_MEM) {
			lvp_event_free(must_handle_ev);
			return ret;
		}
		else {
			if (frame_type == AVMEDIA_TYPE_VIDEO) {
				f->filtered_video_frame = NULL;
			}
			else if (frame_type == AVMEDIA_TYPE_AUDIO)
			{
				f->filtered_audio_frame = NULL;
			}
			lvp_event_free(must_handle_ev);
			av_frame_free(&filtered_frame);
			return ret;
		}
	}


	src_frame = av_frame_clone(ev->data);
	LVPEvent* sub_event = lvp_event_alloc(src_frame, LVP_EVENT_FILTER_GOT_FRAME, LVP_FALSE);
    lvp_event_control_send_event(f->ctl,sub_event);

    //for other core module use
    LVPEvent *must_handle_ev = lvp_event_alloc(sub_event->data,LVP_EVENT_FILTER_SEND_FRAME,LVP_TRUE);
    int ret = lvp_event_control_send_event(f->ctl,must_handle_ev);

	if (ret == LVP_E_NO_MEM) {
		if (frame_type == AVMEDIA_TYPE_VIDEO) {
			f->filtered_video_frame = (AVFrame*)sub_event->data;
		}
		else if (frame_type == AVMEDIA_TYPE_AUDIO)
		{
			f->filtered_audio_frame = (AVFrame*)sub_event->data;
		}
	}
	else {
		src_frame = (AVFrame*)sub_event->data;
		if (frame_type == AVMEDIA_TYPE_VIDEO) {
			f->filtered_video_frame = NULL;
		}
		else if (frame_type == AVMEDIA_TYPE_AUDIO)
		{
			f->filtered_audio_frame = NULL;
		}
		av_frame_free(&src_frame);
	}



	lvp_event_free(must_handle_ev);
	lvp_event_free(sub_event);
    if(ret!=LVP_OK){
        return ret;
    }
    return LVP_OK;
}


static int filter_init(struct lvp_module *module, 
                                    LVPMap *options,
                                    LVPEventControl *ctl,
                                    LVPLog *log){
    assert(module);
    assert(ctl);

    LVPFrameFilter *f = (LVPFrameFilter*)module->private_data;
    f->log = lvp_log_alloc(module->name);
    f->log->log_call = log->log_call;
    f->log->usr_data = log->usr_data;

    f->ctl = ctl;

    int ret = lvp_event_control_add_listener(ctl,LVP_EVENT_DECODER_SEND_FRAME,handle_frame,f);
    if(ret != LVP_OK){
        lvp_error(f->log,"add listener error",NULL);
        return LVP_E_FATAL_ERROR;
    }

    //init sub module
    f->modules = lvp_list_alloc();
    void *op = NULL;
    for (LVPModule *m = NULL; (m = lvp_module_iterate(&op)) != NULL; )
    {
        if(m->type == LVP_MODULE_FRAME_FILTER){
            LVPModule *c = lvp_module_create_module(m);
            if(c!=NULL){
                int ret = lvp_module_init(c,options,ctl,log);
                if(ret != LVP_OK){
                    lvp_module_close(c);
                    lvp_mem_free(c);
                }
                lvp_list_add(f->modules,c,NULL,filter_sub_module_free,1);
            }
        }
    }
    
    return LVP_OK;

}

static void filter_close(struct lvp_module *module){
    assert(module);
    LVPFrameFilter *f = (LVPFrameFilter*)module->private_data;
    lvp_event_control_remove_listener(f->ctl,LVP_EVENT_DECODER_SEND_FRAME,handle_frame,f);
    if(f->modules){
        lvp_list_free(f->modules);
    }
	if (f->log) {
		lvp_log_free(f->log);
	}
	lvp_mem_free(f);
}

LVPModule lvp_frame_filter = {
    .version = lvp_version,
    .name = "LVP_FRAME_FILTER",
    .type = LVP_MODULE_CORE|LVP_MODULE_FRAME_FILTER,
    .private_data_size = sizeof(LVPFrameFilter),
    .private_data = NULL,
    .module_init = filter_init,
    .module_close = filter_close,
};