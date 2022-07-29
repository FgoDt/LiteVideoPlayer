#include "../core/lvp_core.h"
#include <libswresample/swresample.h>

typedef struct lvp_audio_format {
	int channels;
	int format;
	int sample_rate;
}LVPAudioFormat;

typedef struct lvp_audio_resample {
	struct SwrContext* swr;
	LVPEventControl* ctl;
	LVPLog* log;
	LVPAudioFormat target;
	LVPAudioFormat src;
	lvp_mutex mutex;
}LVPAudioResample;

static int create_av_swr(LVPAudioResample* r) {
	if (r->swr) {
		swr_close(r->swr);
		swr_free(&r->swr);
		r->swr = NULL;
	}

	r->swr = swr_alloc_set_opts(NULL, av_get_default_channel_layout(r->target.channels),
		r->target.format, r->target.sample_rate, av_get_default_channel_layout(r->src.channels),
		r->src.format, r->src.sample_rate,0, NULL);
	if (r->swr == NULL) {
		lvp_error(r->log, "audio resample error", NULL);
		return LVP_E_FATAL_ERROR;
	}
	int ret = swr_init(r->swr);
	if (ret < 0) {
		lvp_error(r->log, "swr init error", NULL);

		return LVP_E_FATAL_ERROR;
	}
	return LVP_OK;
}

static int handle_change_format(LVPEvent* ev, void* usrdata) {
	LVPAudioFormat* format = (LVPAudioFormat*)ev->data;
	LVPAudioResample* r = (LVPAudioResample*)usrdata;
	if (format->channels != r->target.channels ||
		format->format != r->target.format ||
		format->sample_rate != r->target.sample_rate)
	{
		lvp_mutex_lock(&r->mutex);

		lvp_mutex_unlock(&r->mutex);
	}
	return LVP_OK;
}

static int handle_frame(LVPEvent* ev, void* usrdata) {
    AVFrame *frame = (AVFrame *) ev->data;

    //video frame
    if (frame->width > 0) {
        return LVP_OK;
    }


    if (usrdata == NULL) {
        return LVP_E_USE_NULL;
    }
    LVPAudioResample *r = (LVPAudioResample *) usrdata;
    if (r->target.format == AV_SAMPLE_FMT_NONE && frame->format > AV_SAMPLE_FMT_DBL) {
        //set planner to signal planner
        //r->target.format = frame->format - 5;
        //TODO use options set fmt
        r->target.format = AV_SAMPLE_FMT_S16;
        //r->target.format = frame->;
        r->target.channels = frame->channels;
        r->target.sample_rate = frame->sample_rate;
    }

    if (r->target.format == AV_SAMPLE_FMT_NONE) {
        return LVP_OK;
    }

    if (frame->format != r->src.format ||
        frame->channels != r->src.channels ||
        frame->sample_rate != r->src.sample_rate) {

        r->src.format = frame->format;
        r->src.channels = frame->channels;
        r->src.sample_rate = frame->sample_rate;

        lvp_mutex_lock(&r->mutex);
        int ret = create_av_swr(r);
        lvp_mutex_unlock(&r->mutex);
        if (ret != LVP_OK) {
            return ret;
        }
    }

    AVFrame *dstframe = av_frame_alloc();
    dstframe->format = r->target.format;
    dstframe->channels = r->target.channels;
    dstframe->sample_rate = r->target.sample_rate;
    dstframe->nb_samples = frame->nb_samples * r->target.sample_rate / frame->sample_rate + 256;
    dstframe->pts = frame->pts;
    dstframe->pkt_dts = frame->pkt_dts;

    int ret = av_frame_get_buffer(dstframe, 1);
    if (ret < 0) {
        lvp_error(r->log, "get buffer error", NULL);
        av_frame_free(&dstframe);
        return LVP_E_FATAL_ERROR;
    }
    lvp_mutex_lock(&r->mutex);
    ret = swr_convert(r->swr, dstframe->data, dstframe->nb_samples,
                      (const uint8_t **) frame->extended_data, frame->nb_samples);
    lvp_mutex_unlock(&r->mutex);
    if (ret <= 0) {
        lvp_error(r->log, "swr convert error", NULL);
        av_frame_free(&dstframe);
        return LVP_E_FATAL_ERROR;
    }
    dstframe->nb_samples = ret;
    dstframe->linesize[0] =
            dstframe->nb_samples * dstframe->channels * av_get_bytes_per_sample(dstframe->format);
    ev->data = dstframe;
    av_frame_free(&frame);
    return LVP_OK;

}


static int filter_init(struct lvp_module* module,
	LVPMap* options,
	LVPEventControl* ctl,
	LVPLog* log) {
	assert(module);
	assert(ctl);
	assert(log);

	LVPAudioResample* r = (LVPAudioResample*)module->private_data;
	r->log = lvp_log_alloc(module->name);
	r->log->log_call = log->log_call;
	r->log->usr_data = log->usr_data;

	r->target.format = AV_SAMPLE_FMT_NONE;
	r->src.format = AV_SAMPLE_FMT_NONE;
	r->ctl = ctl;


	int ret = lvp_event_control_add_listener(r->ctl,LVP_EVENT_FILTER_GOT_FRAME,handle_frame,r);
	if(ret != LVP_OK){
		lvp_error(r->log,"add %s listener error",LVP_EVENT_FILTER_GOT_FRAME);
		return LVP_E_FATAL_ERROR;
	}

	lvp_mutex_create(&r->mutex);

	ret  = lvp_event_control_add_listener(r->ctl,LVP_EVENT_CHANGE_AUDIO_FORMAT,handle_change_format,r);
	if(ret != LVP_OK){
		lvp_error(r->log,"add %s listener error",LVP_EVENT_CHANGE_AUDIO_FORMAT);
		return LVP_E_FATAL_ERROR;
	}

	//todo use options set target format

	return LVP_OK;
}

static void filter_close(struct lvp_module* module){

	LVPAudioResample* r = (LVPAudioResample*)module->private_data;

	lvp_event_control_remove_listener(r->ctl, LVP_EVENT_FILTER_GOT_FRAME, handle_frame, r);
	lvp_mutex_lock(&r->mutex);
	lvp_mutex_unlock(&r->mutex);
	if (r->log) {
		lvp_log_free(r->log);
	}

	if (r->swr) {
		swr_close(r->swr);
		swr_free(&r->swr);
	}
	lvp_mutex_free(&r->mutex);
	lvp_mem_free(r);
	module->private_data = NULL;

}

LVPModule lvp_audio_resample= {
	.version = lvp_version,
	.name = "LVP_AUDIO_RESAMPLE",
	.type = LVP_MODULE_FRAME_FILTER,
	.private_data_size = sizeof(LVPAudioResample),
	.private_data = NULL,
	.module_init = filter_init,
	.module_close = filter_close,
};