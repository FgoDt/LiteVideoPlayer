#include "lvp_decoder.h"
#include <libavformat/avformat.h>
#include "lvp_hwaccel_decoder.h"
#include "../core/lvp_pkt.h"

static void close_decoder(LVPDecoder *decoder){
    if(decoder->decoder_thread_run == 1){
        decoder->decoder_thread_run = 0;
        lvp_thread_join(decoder->dec_thread);
    }
    if(decoder->codec){
        decoder->codec = NULL;
    }
    if(decoder->avctx){
        avcodec_close(decoder->avctx);
        avcodec_free_context(&decoder->avctx);
        decoder->avctx = NULL;
    }
    if(decoder->iframe){
        av_frame_free(&decoder->iframe);
    }
}

static int init_audio_decoder(LVPDecoder *decoder, LVPPkt *pkt){
	lvp_mutex_lock(&decoder->mutex);
	if (decoder->avctx) {
		avcodec_close(decoder->avctx);
		avcodec_free_context(&decoder->avctx);
	}
	lvp_mutex_unlock(&decoder->mutex);

    decoder->codec = avcodec_find_decoder(pkt->codec_id);
    if(decoder->codec == NULL){
        lvp_error(decoder->log,"can not find decoder for codec id %d",pkt->codec_id);
        return LVP_E_FATAL_ERROR;
    }
    decoder->avctx = avcodec_alloc_context3(decoder->codec);
    if(decoder->avctx == NULL){
        lvp_error(decoder->log,"alloc eglContext error",NULL);
        return LVP_E_NO_MEM;
    }
	decoder->avctx->time_base = pkt->time_base;
	decoder->avctx->extradata = av_malloc(pkt->pkt->size);
	decoder->avctx->extradata_size = pkt->pkt->size;
	memcpy(decoder->avctx->extradata, pkt->pkt->data, pkt->pkt->size);

    //todo set options
    int ret = avcodec_open2(decoder->avctx,decoder->codec,NULL);
    if(ret != LVP_OK){
        lvp_error(decoder->log,"open avcodec error",NULL);
        return LVP_E_FATAL_ERROR;
    }
    return LVP_OK;
}

#ifdef USE_HW

static int find_linux_decoder(LVPDecoder *decoder){
    return LVP_E_FATAL_ERROR;
}

static int find_win32_decoder(LVPDecoder *decoder){
    return LVP_E_FATAL_ERROR;
}

static int find_ios_decoder(LVPDecoder *decoder){
    return LVP_E_FATAL_ERROR;
}

static int find_android_decoder(LVPDecoder *decoder){
    return LVP_E_FATAL_ERROR;
}

#endif

//options lvpop_codec
static int find_best_video_decoder(LVPDecoder *decoder, LVPPkt *pkt){
    if(decoder->options){
        const char *force_codec_name = lvp_map_get(decoder->options,"lvp_vcodec");
        if(force_codec_name)
            decoder->codec = avcodec_find_decoder_by_name(force_codec_name);
    }

	

//todo use hwaccel
#if USE_HW
    //use default codec qsv -> nvdia -> dxva2/vaapi -> h264
    //if android mediacodec -> h264
    //if ios videotoolbox -> h264
    if(!decoder->codec){

#ifdef LVP_LINUX

    find_linux_decoder(decoder);

#endif

#ifdef LVP_ANDROID
#endif

#ifdef LVP_IOS
#endif

#ifdef LVP_WIN32

#endif

    }
#endif //use hwaccel

	if (!decoder->codec) {
		decoder->codec = avcodec_find_decoder(pkt->codec_id);
	}

	if (decoder->codec == NULL) {
		return LVP_E_FATAL_ERROR;
	}


    return LVP_OK;

}

static int init_video_decoder(LVPDecoder *decoder, LVPPkt *pkt){
	lvp_mutex_lock(&decoder->mutex);
	if (decoder->avctx) {
		avcodec_close(decoder->avctx);
		avcodec_free_context(&decoder->avctx);
	}
	lvp_mutex_unlock(&decoder->mutex);
    int ret = find_best_video_decoder(decoder, pkt);
    if(ret != LVP_OK){
        lvp_error(decoder->log,"can not find decoder for codec id %d",pkt->codec_id);
        return LVP_E_FATAL_ERROR;
    }

    decoder->avctx = avcodec_alloc_context3(decoder->codec);
	decoder->avctx->time_base = pkt->time_base;
	decoder->avctx->width = pkt->width;
	decoder->avctx->height = pkt->height;
	decoder->avctx->extradata = av_malloc(pkt->pkt->size);
	decoder->avctx->extradata_size = pkt->pkt->size;
	memcpy(decoder->avctx->extradata, pkt->pkt->data, pkt->pkt->size);

	decoder->avctx->thread_count = 2;
	decoder->avctx->thread_type = FF_THREAD_FRAME;
	const char* no_hw = lvp_map_get(decoder->options, "lvpnhw");
	if (no_hw==NULL||strcmp(no_hw,"1"))
	{
		lvp_set_up_hwaccel_decoder(decoder);
	}
    //todo set options
    ret = avcodec_open2(decoder->avctx,decoder->codec,NULL);
    if(ret < 0){
        lvp_error(decoder->log,"avcodec open2 error",NULL);
        return LVP_E_FATAL_ERROR;
    }
    return LVP_OK;
}



static void* decoder_thread(void *data){
    LVPDecoder *d = (LVPDecoder*)data;
    lvp_debug(d->log,"in decoder thread type %d",d->codec_type);
    d->decoder_thread_run = 1;
    LVPEvent *ev = lvp_event_alloc(NULL,LVP_EVENT_REQ_PKT,LVP_TRUE);
    LVPEvent *sev = lvp_event_alloc(NULL,LVP_EVENT_DECODER_SEND_FRAME,LVP_TRUE);
    int need_req = 1;
    d->iframe = av_frame_alloc();
	d->sw_frame = av_frame_alloc();
	int64_t pre_apts = 0;
	int64_t pre_vpts = 0;
	int64_t avg_vduration = 0;
	int64_t avg_aduration = 0;
	int64_t vframes = 0;
	int64_t aframes = 0;
	while (d->decoder_thread_run == 1)
	{
		int ret = 0;
		if (need_req == 1) {
			ev->data = &d->codec_type;
			ret = lvp_event_control_send_event(d->ctl, ev);
			if (ret == LVP_E_NO_MEM) {
				lvp_sleep(10);
				continue;
			}
			else if (ret != LVP_OK)
			{
				lvp_error(d->log, "req pkt error", NULL);
				break;
			}
			if(ev->data == &d->codec_type){
				lvp_error(d->log,"req pkt error",NULL);
				break;
			}
		}
		LVPPkt *lpkt = (LVPPkt*)ev->data;
		d->ipkt = lpkt->pkt;
		if (lpkt->extra_data) {
			lvp_debug(d->log, "reset codec type: %d\n", lpkt->type);
			if (lpkt->type == AVMEDIA_TYPE_AUDIO) {
				ret = init_audio_decoder(d, lpkt);
				if (ret != LVP_OK) {
					lvp_error(d->log, "reset audio codec error\n", NULL);
					break;
				}
				continue;
			}
			else if (lpkt->type == AVMEDIA_TYPE_VIDEO) {
				init_video_decoder(d, lpkt);
				if (ret != LVP_OK) {
					lvp_error(d->log, "reset audio codec error\n", NULL);
					break;
				}
				continue;
			}
		}
		lvp_mutex_lock(&d->mutex);
		ret = avcodec_send_packet(d->avctx, d->ipkt);
		lvp_mutex_unlock(&d->mutex);
		if (ret < 0 && ret != AVERROR(EAGAIN)) {
			lvp_error(d->log, "send packet error %d", ret);
			av_packet_free(&d->ipkt);
			break;
		}
		else if (ret == AVERROR(EAGAIN)) {
			need_req = 0;
		}
		else {
			av_packet_free(&d->ipkt);
			need_req = 1;
		}

		lvp_mutex_lock(&d->mutex);
		ret = avcodec_receive_frame(d->avctx, d->iframe);
		lvp_mutex_unlock(&d->mutex);
		if (ret < 0 && ret != AVERROR(EAGAIN)) {
			lvp_error(d->log, "receive frame error %d", ret);
			break;
		}
		//check pts if NOPTS
		if(d->iframe->pts == AV_NOPTS_VALUE){
			if(d->iframe->width>0){
				vframes++;
				pre_vpts += pre_vpts/vframes; 
				d->iframe->pts = pre_vpts;
			}else{
				aframes++;
				pre_apts += pre_apts/aframes;
				d->iframe->pts = pre_apts;
			}
		}else{
			if(d->iframe->width>0){
				pre_vpts = d->iframe->pts;
				vframes++;
			}else{
				aframes++;
				pre_apts = d->iframe->pts;
			}
		}

//		if(d->iframe->width>0){
//			printf("vpts:%lld duration:%lld\n",d->iframe->pts, d->iframe->pkt_duration);
//		}else{
//			printf("apts:%lld\n",d->iframe->pts);
//		}

		if (ret == 0) {
			sev->data = d->iframe;
			//video
			if (d->iframe->width > 0) {
				if (d->hw_pix_fmt == d->iframe->format) {
					av_frame_unref(d->sw_frame);
					int ret = av_hwframe_transfer_data(d->sw_frame, d->iframe, 0);
					if (ret < 0) {
						lvp_error(d->log, "hwframe transer error", NULL);
						break;
					}
					av_frame_copy_props(d->sw_frame, d->iframe);
					sev->data = d->sw_frame;

					LVPSENDEVENT(d->ctl, LVP_EVENT_DECODER_GOT_FRAME, d->sw_frame);
				}
				else {
					LVPSENDEVENT(d->ctl, LVP_EVENT_DECODER_GOT_FRAME, d->iframe);
				}
			}
			else
			{
				LVPSENDEVENT(d->ctl, LVP_EVENT_DECODER_GOT_FRAME, d->iframe);
			}
		retry:
			ret = lvp_event_control_send_event(d->ctl, sev);
			if (ret != LVP_OK && ret != LVP_E_NO_MEM) {
				break;
			}
			if (ret == LVP_E_NO_MEM && d->decoder_thread_run == 1) {
				lvp_sleep(50);
				goto retry;
			}
		}
	}

	d->decoder_thread_run = 0;
    
    lvp_event_free(ev);
    lvp_event_free(sev);

    lvp_debug(d->log,"out decoder thread type %d",d->codec_type);
    return NULL;
}

static void* sub_decoder_thread(void* data) {
	LVPDecoder* d = (LVPDecoder*)data;
	lvp_debug(d->log, "in decoder thread type %d", d->codec_type);
	d->decoder_thread_run = 1;
	LVPEvent* ev = lvp_event_alloc(NULL, LVP_EVENT_REQ_PKT, LVP_TRUE);
	LVPEvent* sev = lvp_event_alloc(NULL, LVP_EVENT_DECODER_SEND_SUB, LVP_TRUE);
	int need_req = 1;
	while (d->decoder_thread_run == 1)
	{
		int ret = 0;
		if (need_req == 1) {
			ev->data = &d->codec_type;
			ret = lvp_event_control_send_event(d->ctl, ev);
			if (ret == LVP_E_NO_MEM) {
				//字幕不需要太高的刷新率
				lvp_sleep(500);
				continue;
			}
			else if (ret != LVP_OK)
			{
				lvp_error(d->log, "req pkt error", NULL);
				break;
			}
		}

        d->ipkt = (AVPacket*)ev->data;
		AVSubtitle *sub = lvp_mem_mallocz(sizeof(*sub));
		int got = 0;
		ret = avcodec_decode_subtitle2(d->avctx, sub, &got, d->ipkt);
		if (got == 0)
		{
			continue;
		}
		if(sub->start_display_time == 0 && sub->end_display_time == 0){
			sub->start_display_time = d->ipkt->pts;
			sub->end_display_time = d->ipkt->dts+d->ipkt->pts;
			sub->pts = d->ipkt->pts;
		}

		need_req = 1;
		sev->data = sub;
		retry:
		ret = lvp_event_control_send_event(d->ctl,sev);
		if (ret != LVP_OK && ret != LVP_E_NO_MEM) {
			break;
		}
		if (ret == LVP_E_NO_MEM && d->decoder_thread_run == 1) {
			lvp_sleep(500);
			goto retry;
		}
	}

	d->decoder_thread_run = 0;

	lvp_event_free(ev);
	lvp_event_free(sev);

	lvp_debug(d->log, "out decoder thread type %d", d->codec_type);
	return NULL;
}

static int init_sub_decoder(LVPDecoder* decoder) {
	AVStream* stream = decoder->stream;
	decoder->codec = avcodec_find_decoder(stream->codecpar->codec_id);
	if (decoder->codec == NULL) {
		lvp_error(decoder->log, "can not find decoder for codec id %d", stream->codecpar->codec_id);
		return LVP_E_FATAL_ERROR;
	}
	decoder->avctx = avcodec_alloc_context3(decoder->codec);
	if (decoder->avctx == NULL) {
		lvp_error(decoder->log, "alloc eglContext error", NULL);
		return LVP_E_NO_MEM;
	}
	avcodec_parameters_to_context(decoder->avctx, stream->codecpar);

	int ret = avcodec_open2(decoder->avctx, decoder->codec, NULL);
	if (ret != LVP_OK) {
		lvp_error(decoder->log, "open avcodec error", NULL);
		return LVP_E_FATAL_ERROR;
	}
	return LVP_OK;

}

static int handle_select_stream(LVPEvent *ev, void *usrdata){
    LVPDecoder *decoder = (LVPDecoder*)usrdata;
    AVStream *stream = (AVStream*)ev->data;
    if(stream->codecpar->codec_type != decoder->codec_type){
        return LVP_OK;
    }
    close_decoder(decoder);

    decoder->stream = stream;

    int ret = 0;
    if(decoder->codec_type == AVMEDIA_TYPE_AUDIO){
        ret = init_audio_decoder(decoder, NULL);
    }else if(decoder->codec_type == AVMEDIA_TYPE_VIDEO){
        ret = init_video_decoder(decoder, NULL);
	}
	else if (decoder->codec_type == AVMEDIA_TYPE_SUBTITLE) {
		ret = init_sub_decoder(decoder);
		ret |= lvp_thread_create(&decoder->dec_thread,sub_decoder_thread,decoder);
		return ret;
	}

    if(ret != LVP_OK){
        return ret;
    }

    ret = lvp_thread_create(&decoder->dec_thread,decoder_thread,decoder);
    if(ret != LVP_TRUE){
        lvp_error(decoder->log,"create decoder thread error",NULL);
        return LVP_E_FATAL_ERROR;
    }
    return LVP_OK;
}

static int handle_close(LVPEvent *ev, void *usrdata){
    //LVPDecoder *decoder = (LVPDecoder*)usrdata;
    return LVP_OK;
}

static int handle_open(LVPEvent *ev, void *usrdata){
    //LVPDecoder *decoder = (LVPDecoder*)usrdata;
    return LVP_OK;
}



static int module_init(struct lvp_module *module, 
                                    LVPMap *options,
                                    LVPEventControl *ctl,
                                    LVPLog *log){
    assert(module);
    assert(ctl);
    assert(log);

    LVPDecoder *decoder = (LVPDecoder*)module->private_data;
    decoder->ctl = ctl;
    decoder->log = lvp_log_alloc(module->name);
    decoder->log->log_call = log->log_call;
    decoder->log->usr_data = log->usr_data;
    decoder->options = options;
	decoder->hw_pix_fmt = AV_PIX_FMT_NONE;
	lvp_mutex_create(&decoder->mutex);

    //int ret = lvp_event_control_add_listener(ctl,LVP_EVENT_SELECT_STREAM,handle_select_stream,decoder);
    //if(ret != LVP_OK){
    //    lvp_error(decoder->log,"add %s listener error",LVP_EVENT_SELECT_STREAM);
    //    return LVP_E_FATAL_ERROR;
	//}

    int ret = lvp_event_control_add_listener(ctl,LVP_EVENT_CLOSE,handle_close,decoder);
    if(ret != LVP_OK){
        lvp_error(decoder->log,"add %s listener error",LVP_EVENT_CLOSE);
        return LVP_E_FATAL_ERROR;
    }
    ret = lvp_event_control_add_listener(ctl,LVP_EVENT_OPEN,handle_open,decoder);
    if(ret != LVP_OK){
        lvp_error(decoder->log,"add %s listener error",LVP_EVENT_CLOSE);
        return LVP_E_FATAL_ERROR;
    }

    if (!strcmp(module->name,"LVP_AUDIO_DECODER"))
    {
        decoder->codec_type = AVMEDIA_TYPE_AUDIO;
    }else if(!strcmp(module->name,"LVP_VIDEO_DECODER")){
        decoder->codec_type = AVMEDIA_TYPE_VIDEO;
    }else if(!strcmp(module->name,"LVP_SUB_DECODER")){
		decoder->codec_type = AVMEDIA_TYPE_SUBTITLE;
	}
	else
	{
        return LVP_E_NO_MEDIA;
	}

    ret = lvp_thread_create(&decoder->dec_thread,decoder_thread,decoder);
    if(ret != LVP_TRUE){
        lvp_error(decoder->log,"create decoder thread error",NULL);
        return LVP_E_FATAL_ERROR;
    }


    return LVP_OK;
}

static void module_close(struct lvp_module *module){
    assert(module);
    LVPDecoder *decoder = (LVPDecoder*)module->private_data;
	if (decoder->decoder_thread_run == 1) {
		decoder->decoder_thread_run = 0;
		lvp_thread_join(decoder->dec_thread);
	}
	lvp_mutex_lock(&decoder->mutex);
	lvp_mutex_unlock(&decoder->mutex);
	lvp_mutex_free(&decoder->mutex);

	if (decoder->avctx != NULL) {
		avcodec_close(decoder->avctx);
		avcodec_free_context(&decoder->avctx);
	}

	if (decoder->hw_device_ctx) {
		av_buffer_unref(&decoder->hw_device_ctx);
	}

	if (decoder->iframe) {
		av_frame_free(&decoder->iframe);
	}

	if (decoder->sw_frame) {
		av_frame_free(&decoder->sw_frame);
	}

	if (decoder->ipkt) {
		av_packet_free(&decoder->ipkt);
	}
	if (decoder->log) {
		lvp_log_free(decoder->log);
	}
	lvp_mem_free(decoder);
    return ;
}

LVPModule lvp_audio_decoder = {
    .version = lvp_version,
    .name = "LVP_AUDIO_DECODER",
    .type = LVP_MODULE_CORE|LVP_MODULE_DECODER,
    .private_data_size = sizeof(LVPDecoder),
    .private_data = NULL,
    .module_init = module_init, 
    .module_close = module_close,
};

LVPModule lvp_video_decoder = {
    .version = lvp_version,
    .name = "LVP_VIDEO_DECODER",
    .type = LVP_MODULE_CORE|LVP_MODULE_DECODER,
    .private_data_size = sizeof(LVPDecoder),
    .private_data = NULL,
    .module_init = module_init, 
    .module_close = module_close,
};

LVPModule lvp_sub_decoder = {
    .version = lvp_version,
    .name = "LVP_SUB_DECODER",
    .type = LVP_MODULE_CORE|LVP_MODULE_DECODER,
    .private_data_size = sizeof(LVPDecoder),
    .private_data = NULL,
    .module_init = module_init, 
    .module_close = module_close,
};