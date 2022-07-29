//
// Created by fgodt on 2022/7/20.
//

#include "../../../../src/core/lvp_module.h"
#include "../../../../src/core/lvp.h"
#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>
#include <libavcodec/avcodec.h>
#include "../../../../src/core/lvp_nqueue.h"
#include <jni.h>



typedef struct LVPAndroidAudioRender{
    LVPEventControl *ctl;
    LVPLog *log;

    uint64_t audio_delay;
    int sl_player_inited;

    //sles data
    SLObjectItf sl_engine_object;
    SLEngineItf sl_engine;

    SLObjectItf sl_output_mix_object;
    SLEnvironmentalReverbItf sl_output_mix_environmental_reverb;

    SLObjectItf sl_player_object;
    SLPlayItf sl_play;
    SLAndroidSimpleBufferQueueItf sl_player_buffer_queue;
    SLVolumeItf sl_player_volume;
    SLmilliHertz sl_player_sample_rate;
    int sl_player_buf_size;

    LVPNQueue *queue;

    lvp_mutex mutex;

    JNIEnv *env;
    JavaVM *vm;
    jobject audio_track_render;
    jmethodID audio_track_render_on_update_mid;
}LVPAndroidAudioRender;

static const SLEnvironmentalReverbSettings reverbSettings =
        SL_I3DL2_ENVIRONMENT_PRESET_STONECORRIDOR;



static void sl_player_callback(SLAndroidSimpleBufferQueueItf q, void *context) {
    LVPAndroidAudioRender *r = context;
    lvp_mutex_lock(&r->mutex);
    AVFrame *frame = lvp_nqueue_pop(r->queue);
    lvp_mutex_unlock(&r->mutex);
    if(frame!= NULL) {
        SLresult ret = (*r->sl_player_buffer_queue)->Enqueue(r->sl_player_buffer_queue,
                                                             frame->data[0],
                                                             frame->linesize[0]);
        av_frame_free(&frame);
        if (ret != SL_RESULT_SUCCESS) {
            lvp_waring(r->log, "SL bufffer queue enqueue error: %d", ret);
        }
    }else{
        uint8_t *tmp = lvp_mem_mallocz(1024*2*2);
        (*r->sl_player_buffer_queue)->Enqueue(r->sl_player_buffer_queue,tmp, 1024*2*2);
        //lvp_waring(r->log, "audio lag", NULL);
    }
    //  lvp_debug(r->log, "SL CALLBACK", NULL);
}

static int sl_set_play_status(LVPAndroidAudioRender *r){
    //need call once by user
    sl_player_callback(r->sl_player_buffer_queue, r);

    SLresult result = (*r->sl_play)->SetPlayState(r->sl_play, SL_PLAYSTATE_PLAYING);
    if (SL_RESULT_SUCCESS != result) {
        lvp_error(r->log, "sl set play state playing error\n", NULL);
        return LVP_E_FATAL_ERROR;
    }
    return LVP_OK;
}

static int create_sles_player(LVPAndroidAudioRender *r, AVFrame *frame) {
    SLresult result;
    r->sl_player_sample_rate = frame->sample_rate;

    SLDataLocator_AndroidBufferQueue loc_bufq = {SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE, 2};
    SLDataFormat_PCM format_pcm = {SL_DATAFORMAT_PCM, frame->channels,
                                   frame->sample_rate * 1000,
                                   SL_PCMSAMPLEFORMAT_FIXED_16,
                                   SL_PCMSAMPLEFORMAT_FIXED_16,
                                   SL_SPEAKER_FRONT_LEFT | SL_SPEAKER_FRONT_RIGHT,
                                   SL_BYTEORDER_LITTLEENDIAN};
    SLDataSource audio_src = {&loc_bufq, &format_pcm};

    SLDataLocator_OutputMix loc_outmix = {SL_DATALOCATOR_OUTPUTMIX, r->sl_output_mix_object};
    SLDataSink audio_sink = {&loc_outmix, NULL};
    const SLInterfaceID ids[2] = {SL_IID_BUFFERQUEUE, SL_IID_VOLUME};
    const SLboolean req[2] = {SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE};
    result = (*r->sl_engine)->CreateAudioPlayer(r->sl_engine, &r->sl_player_object, &audio_src,
                                                &audio_sink, 2, ids, req);
    if (SL_RESULT_SUCCESS != result) {
        lvp_error(r->log, "sl engine create audio player error\n", NULL);
        return LVP_E_FATAL_ERROR;
    }

    result = (*r->sl_player_object)->Realize(r->sl_player_object, SL_BOOLEAN_FALSE);
    if (SL_RESULT_SUCCESS != result) {
        lvp_error(r->log, "sl player realize error\n", NULL);
        return LVP_E_FATAL_ERROR;
    }

    result = (*r->sl_player_object)->GetInterface(r->sl_player_object, SL_IID_PLAY, &r->sl_play);
    if (SL_RESULT_SUCCESS != result) {
        lvp_error(r->log, "sl get play interface realize error\n", NULL);
        return LVP_E_FATAL_ERROR;
    }

    result = (*r->sl_player_object)->GetInterface(r->sl_player_object, SL_IID_BUFFERQUEUE,
                                                  &r->sl_player_buffer_queue);
    if (SL_RESULT_SUCCESS != result) {
        lvp_error(r->log, "sl get buffer queue interface error\n", NULL);
        return LVP_E_FATAL_ERROR;
    }


    result = (*r->sl_player_buffer_queue)->RegisterCallback(r->sl_player_buffer_queue, sl_player_callback,
                                                      r);
    if (SL_RESULT_SUCCESS != result) {
        lvp_error(r->log, "sl register buffer queue callback error\n", NULL);
        return LVP_E_FATAL_ERROR;
    }

    result = (*r->sl_player_object)->GetInterface(r->sl_player_object, SL_IID_VOLUME,
                                                  &r->sl_player_volume);
    if (SL_RESULT_SUCCESS != result) {
        lvp_error(r->log, "sl get volume interface error\n", NULL);
        return LVP_E_FATAL_ERROR;
    }





    lvp_debug(r->log, "create audio player done", NULL);
    return LVP_OK;
}

static int sl_init(LVPAndroidAudioRender *r){
    SLresult ret;
    ret = slCreateEngine(&r->sl_engine_object, 0, NULL, 0, NULL, NULL);
    if (SL_RESULT_SUCCESS != ret) {
        lvp_error(r->log, "sl create engine error\n", NULL);
        return LVP_E_FATAL_ERROR;
    }


    ret = (*r->sl_engine_object)->Realize(r->sl_engine_object, SL_BOOLEAN_FALSE);
    if (SL_RESULT_SUCCESS != ret) {
        lvp_error(r->log, "sl engine object realize error\n", NULL);
        return LVP_E_FATAL_ERROR;
    }


    ret = (*r->sl_engine_object)->GetInterface(r->sl_engine_object, SL_IID_ENGINE, &r->sl_engine);
    if (SL_RESULT_SUCCESS != ret) {
        lvp_error(r->log, "sl get engine interface error\n", NULL);
        return LVP_E_FATAL_ERROR;
    }


    const SLInterfaceID ids[1] = {SL_IID_ENVIRONMENTALREVERB};
    const SLboolean req[1] = {SL_BOOLEAN_FALSE};
    ret = (*r->sl_engine)->CreateOutputMix(r->sl_engine, &r->sl_output_mix_object, 1, ids, req);
    if (SL_RESULT_SUCCESS != ret) {
        lvp_error(r->log, "sl create output mix error\n", NULL);
        return LVP_E_FATAL_ERROR;
    }

    ret = (*r->sl_output_mix_object)->Realize(r->sl_output_mix_object, SL_BOOLEAN_FALSE);
    if (SL_RESULT_SUCCESS != ret) {
        lvp_error(r->log, "sl mix realize error\n", NULL);
        return LVP_E_FATAL_ERROR;
    }

    //ret = (*r->sl_output_mix_object)->GetInterface(r->sl_output_mix_object, SL_IID_ENVIRONMENTALREVERB, &r->sl_output_mix_environmental_reverb);
    //if (SL_RESULT_SUCCESS != ret) {
    //    (*r->sl_output_mix_environmental_reverb)->SetEnvironmentalReverbProperties(
    //            r->sl_output_mix_environmental_reverb,&reverbSettings
    //            );
    //}

    r->queue = lvp_nqueue_alloc(2);
    if(!r->queue){
        lvp_error(r->log, "nqueue alloc error", NULL);
        return LVP_E_FATAL_ERROR;
    }


    return LVP_OK;
}


static int handle_audio_update(LVPEvent* e, void *usrdata){
    LVPAndroidAudioRender *r = (LVPAndroidAudioRender*)usrdata;

    AVFrame *frame = e->data;

    //return LVP_OK;

    if(r->queue->size == r->queue->cap){
        return  LVP_E_NO_MEM;
    }

    lvp_mutex_lock(&r->mutex);
    AVFrame *c_frame = av_frame_clone(frame);
    lvp_nqueue_push(r->queue, c_frame, NULL, NULL, LVP_FALSE);
    lvp_mutex_unlock(&r->mutex);

    if(!r->sl_player_inited){
        r->sl_player_inited = 1;
        create_sles_player(r, frame);
        sl_set_play_status(r);
    }

    SLAndroidBufferQueueState  state = {0};
    if(r->sl_player_buffer_queue){
        (*r->sl_player_buffer_queue)->GetState(r->sl_player_buffer_queue, &state);
    }

    r->audio_delay = (r->queue->size + state.count - 1)*1000*frame->nb_samples/frame->sample_rate;
    e->response = &r->audio_delay;
    return  LVP_OK;
}

static int module_init(struct lvp_module *module,
        LVPMap *options,
        LVPEventControl *ctl,
        LVPLog *log){

    LVPAndroidAudioRender  *r = (LVPAndroidAudioRender*)module->private_data;
    r->ctl = ctl;
    r->log = lvp_log_alloc(module->name);
    r->log->log_call = log->log_call;
    r->log->usr_data = log->usr_data;


    int ret = lvp_event_control_add_listener(r->ctl, LVP_EVENT_UPDATE_AUDIO, handle_audio_update, r);
    if(ret != LVP_OK){
        lvp_error(r->log, "add update audio listener error", NULL);
        return LVP_E_FATAL_ERROR;
    }

    ret = sl_init(r);
    if(ret != LVP_OK){
        lvp_error(r->log, "opensles init error", NULL);
        return LVP_E_FATAL_ERROR;
    }

    return LVP_OK;
}

static void module_close(struct lvp_module *module){

}

LVPModule lvp_android_audio_render = {
        .version = lvp_version,
        .name = "LVP_ANDROID_AUDIO_RENDER",
        .type = LVP_MODULE_CORE|LVP_MODULE_RENDER,
        .private_data_size = sizeof(LVPAndroidAudioRender),
        .private_data = NULL,
        .module_init = module_init,
        .module_close = module_close,
};
