#include "lvp_avsync.h"
#include <libavformat/avformat.h>
#include <libavutil/time.h>

static void got_frame(LVPAVSync *sync){
    if(sync->has_audio == 1){
        if(sync->aframe == NULL){
            int type = AVMEDIA_TYPE_AUDIO;
            sync->req_event->data = &type;
            int ret = lvp_event_control_send_event(sync->ctl,sync->req_event);
            if(ret == LVP_OK){
                sync->aframe = (AVFrame*)sync->req_event->data;
            }
        }
    }

    if(sync->has_video == 1){
        if(sync->vframe == NULL){
            int type = AVMEDIA_TYPE_VIDEO;
            sync->req_event->data = &type;
            int ret = lvp_event_control_send_event(sync->ctl,sync->req_event);
            if(ret == LVP_OK){
                sync->vframe = (AVFrame*)sync->req_event->data;
            }
        }
    }

	if (sync->has_sub == 1) {
		if (sync->sub == NULL)
		{
			int ret = lvp_event_control_send_event(sync->ctl, sync->req_sub_event);
			if (ret == LVP_OK) {
				sync->sub = (AVSubtitle*)sync->req_sub_event->data;
			}
		}
	}

}

static void sync_audio_master(LVPAVSync *sync){
    got_frame(sync);
    if(sync->aframe != NULL){
		//printf("%ld\r", sync->aframe->pts);
		//if (sync->seeked == 1 && check_is_seek_frame(sync->aframe->pts,sync->seek_time) != 1)
		//{
		//	av_frame_unref(sync->aframe);
		//	av_frame_free(&sync->aframe);
		//	sync->aframe = NULL;
		//	return;
		//}
		sync->seeked = 0;
        sync->update_audio_event->data = sync->aframe;
        int ret = lvp_event_control_send_event(sync->ctl,sync->update_audio_event);

        uint64_t buf_time = 0;
		if(sync->update_audio_event->response) {
			buf_time = *(uint64_t *) sync->update_audio_event->response;
			if(buf_time<0){
				buf_time = 0;
			}
		}
		sync->audio_time = sync->aframe->pts - buf_time ;
        if(ret == LVP_OK){
            av_frame_unref(sync->aframe);
            av_frame_free(&sync->aframe);
            sync->aframe = NULL;
        }else if(ret != LVP_E_NO_MEM){
            lvp_error(sync->log,"audio render return error %d",ret);
            sync->sync_run = 0;
            return;
        }
    }
    if(sync->vframe != NULL && sync->vframe->pts + sync->vframe->pkt_duration <= sync->audio_time){
        sync->update_video_event->data = sync->vframe;
        int ret = lvp_event_control_send_event(sync->ctl,sync->update_video_event);
        if(ret == LVP_OK){
			printf("A-V:%06llud\r", sync->audio_time - sync->vframe->pts);
            av_frame_unref(sync->vframe);
            av_frame_free(&sync->vframe);
            sync->vframe = NULL;
        }else if(ret != LVP_E_NO_MEM){
            lvp_error(sync->log,"video render return error %d",ret);
            // exit loop
            sync->sync_run = 0;
            return;
        }
	}
	else if (sync->vframe != NULL && (sync->vframe->pts) > sync->audio_time + 10000) {
		av_frame_unref(sync->vframe);
		av_frame_free(&sync->vframe);
		sync->vframe = NULL;
	}
	if (sync->sub != NULL && sync->sub->pts <= sync->audio_time)
	{
		sync->update_sub_event->data = sync->sub;
		int ret = lvp_event_control_send_event(sync->ctl, sync->update_sub_event);
		if (ret == LVP_OK) {
			avsubtitle_free(sync->sub);
			lvp_mem_free(sync->sub);
			sync->sub = NULL;
		}
		else if (ret != LVP_E_NO_MEM) {
			lvp_error(sync->log, "sub render return error %d", ret);
			sync->sync_run = 0;
			return;
		}
	}
	else if (sync->sub != NULL && (sync->sub->pts - 1000) > sync->audio_time) {
			avsubtitle_free(sync->sub);
			lvp_mem_free(sync->sub);
			sync->sub = NULL;
	}
}

//when media no audio then we use extra time to sync video 
static void sync_video_master(LVPAVSync *sync){
    got_frame(sync);
    if(sync->vframe!= NULL){
		//if (sync->seeked == 1 && check_is_seek_frame(sync->vframe->pts,sync->seek_time) != 1)
		//{
		//	av_frame_unref(sync->vframe);
		//	av_frame_free(&sync->vframe);
		//	sync->vframe = NULL;
		//	return;
		//}
		sync->seeked = 0;
		if (sync->vframe->pts < 0 || sync->vframe->pts <= sync->ex_time) {
			sync->update_video_event->data = sync->vframe;
			int ret = lvp_event_control_send_event(sync->ctl, sync->update_video_event);
			if (ret == LVP_OK) {
				av_frame_unref(sync->vframe);
				av_frame_free(&sync->vframe);
				sync->vframe = NULL;
			}
			else if (ret != LVP_E_NO_MEM) {
				lvp_error(sync->log, "video render return error %d", ret);
				// exit loop
				sync->sync_run = 0;
				return;
			}
		}
		else if ((sync->vframe->pts - 1000) > sync->ex_time) {
				av_frame_unref(sync->vframe);
				av_frame_free(&sync->vframe);
				sync->vframe = NULL;
		}
    }
	if (sync->sub != NULL && sync->sub->pts <= sync->ex_time)
	{
		sync->update_sub_event->data = sync->sub;
		int ret = lvp_event_control_send_event(sync->ctl, sync->update_sub_event);
		if (ret == LVP_OK) {
			avsubtitle_free(sync->sub);
			lvp_mem_free(sync->sub);
			sync->sub = NULL;
		}
		else if (ret != LVP_E_NO_MEM) {
			lvp_error(sync->log, "sub render return error %d", ret);
			sync->sync_run = 0;
			return;
		}
	}
	else if (sync->sub != NULL && (sync->sub->pts - 1000) > sync->audio_time) {
		avsubtitle_free(sync->sub);
		lvp_mem_free(sync->sub);
		sync->sub = NULL;
	}
}

static void *sync_thread(void *data){
    LVPAVSync *sync = (LVPAVSync*)data;
    sync->sync_run = 1;
    
    sync->req_event = lvp_event_alloc(NULL,LVP_EVENT_REQ_FRAME,LVP_TRUE);
    sync->req_sub_event = lvp_event_alloc(NULL,LVP_EVENT_REQ_SUB,LVP_TRUE);
    sync->update_audio_event = lvp_event_alloc(NULL,LVP_EVENT_UPDATE_AUDIO,LVP_TRUE);
    sync->update_video_event = lvp_event_alloc(NULL,LVP_EVENT_UPDATE_VIDEO,LVP_TRUE);
    sync->update_sub_event = lvp_event_alloc(NULL,LVP_EVENT_UPDATE_SUB,LVP_TRUE);
    uint64_t time = av_gettime()/1000;
    while (sync->sync_run == 1)
    {
		if (sync->pause == 1)
		{
			lvp_sleep(30);
			continue;
		}
		int synctype = sync->has_audio  == 1? AVMEDIA_TYPE_AUDIO:AVMEDIA_TYPE_VIDEO;
        if(synctype == AVMEDIA_TYPE_AUDIO){
            sync_audio_master(sync);
        }else{
            sync_video_master(sync);
        }
        lvp_sleep(3);
        sync->ex_time = av_gettime()/1000 - time;
    }
    return NULL;
}

static int handle_select_stream(LVPEvent *ev, void *usrdata){
    assert(ev);
    assert(usrdata);
    AVStream *stream  = (AVStream*)ev->data;
    LVPAVSync *sync = (LVPAVSync*)usrdata;
    if(stream->codecpar->codec_type == AVMEDIA_TYPE_AUDIO ){
        sync->has_audio = 1;
    }else if(stream->codecpar->codec_type == AVMEDIA_TYPE_VIDEO){
        sync->has_video = 1;
	}
	else if (stream->codecpar->codec_type == AVMEDIA_TYPE_SUBTITLE) {
		sync->has_sub = 1;
	}
    return LVP_OK;
}

static int handle_pause(LVPEvent* ev, void* usrdata) {
	assert(ev);
	assert(usrdata);
    LVPAVSync *sync = (LVPAVSync*)usrdata;
	sync->pause = 1;
	return LVP_OK;
}

static int handle_resume(LVPEvent* ev, void* usrdata) {
	assert(ev);
	assert(usrdata);
	LVPAVSync* sync = (LVPAVSync*)usrdata;
	sync->pause = 0;
	return LVP_OK;
}

static int handle_seek(LVPEvent* ev, void* usrdata) {
	assert(ev);
	assert(usrdata);
	LVPAVSync* sync = (LVPAVSync*)usrdata;
	int64_t seek_time = *(int64_t*)ev->data;
	sync->seeked = 1; ///< not use
	sync->seek_time = seek_time; ///< not use
	sync->ex_time = seek_time / 1000;
	return LVP_OK;
}

static int module_init(struct lvp_module *module, 
                                    LVPMap *options,
                                    LVPEventControl *ctl,
                                    LVPLog *log){
    assert(module);
    assert(log);
    assert(ctl);
    LVPAVSync *sync = (LVPAVSync*)module->private_data;
    sync->ctl = ctl;
    sync->log = lvp_log_alloc(module->name);
    sync->log->log_call = log->log_call;
    sync->log->usr_data = log->usr_data;

    int ret = lvp_event_control_add_listener(ctl,LVP_EVENT_SELECT_STREAM,handle_select_stream,sync);

    if(ret!=LVP_OK){
        return LVP_E_FATAL_ERROR;
    }

	ret = lvp_event_control_add_listener(ctl, LVP_EVENT_PAUSE, handle_pause, sync);
    if(ret!=LVP_OK){
        return LVP_E_FATAL_ERROR;
    }

	ret = lvp_event_control_add_listener(ctl, LVP_EVENT_RESUME, handle_resume, sync);
    if(ret!=LVP_OK){
        return LVP_E_FATAL_ERROR;
    }

	ret = lvp_event_control_add_listener(ctl, LVP_EVENT_SEEK, handle_seek, sync);
    if(ret!=LVP_OK){
        return LVP_E_FATAL_ERROR;
    }

    ret = lvp_thread_create(&sync->sync_thread_t,sync_thread,sync);

    return ret == LVP_FALSE ? LVP_E_FATAL_ERROR : LVP_OK;
}


static void module_close(struct lvp_module *module){
	LVPAVSync* sync = (LVPAVSync*)module->private_data;
	if (sync->sync_run == 1) {
		sync->sync_run = 0;
		lvp_thread_join(sync->sync_thread_t);
	}
	if (sync->log) {
		lvp_log_free(sync->log);
	}
	if (sync->req_event) {
		lvp_event_free(sync->req_event);
	}
	if(sync->req_sub_event){
		lvp_event_free(sync->req_sub_event);
	}
	if (sync->update_audio_event) {
		lvp_event_free(sync->update_audio_event);
	}
	if (sync->update_video_event) {
		lvp_event_free(sync->update_video_event);
	}
	if (sync->update_sub_event){
		lvp_event_free(sync->update_sub_event);
	}
	if (sync->aframe) {
		av_frame_free(&sync->aframe);
	}
	if (sync->vframe) {
		av_frame_free(&sync->vframe);
	}
	if (sync->sub) {
		avsubtitle_free(sync->sub);
		lvp_mem_free(sync->sub);
	}
	lvp_mem_free(sync);
}

LVPModule lvp_avsync_module = {
    .version = lvp_version,
    .name = "LVP_AVSYNC_MODULE",
    .type = LVP_MODULE_CORE,
    .private_data_size = sizeof(LVPAVSync),
    .private_data = NULL,
    .module_init = module_init,
    .module_close = module_close,
};