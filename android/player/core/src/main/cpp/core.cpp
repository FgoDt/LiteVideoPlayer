#include <jni.h>
#include <string>

#include <jni.h>
#include <string>
#include <android/native_window_jni.h>
#include <android/log.h>
#include <libavcodec/avcodec.h>
extern "C" {
#include "../../../../../../src/core/lvp_core.h"
#include "../../../../../../src/core/lvp.h"
}

static const char* LVP_ANDROID_AUDIO_RENDER = "LVP_ANDROID_AUDIO_RENDER";
static const char* LVP_ANDROID_AUDIO_TRACK_RENDER = "LVP_ANDROID_AUDIO_TRACK_RENDER";
static const char* LVP_ANDROID_VIDEO_RENDER = "LVP_ANDROID_VIDEO_RENDER";

static void android_log_call(const char* log, void *data){
    __android_log_print(ANDROID_LOG_VERBOSE, "LVPJNI", "%s", log);
}

typedef struct LVPAudioTrackRender{
    LVPEventControl *ctl;
    LVPLog *log;
    uint64_t delay;

    JNIEnv *env;
    JavaVM *vm;
    jobject audio_track_render;
    jmethodID audio_track_render_on_update_mid;
}LVPAudioTrackRender;

static int audio_track_handle_update_audio(LVPEvent *e, void *data){
    auto r = (LVPAudioTrackRender*)data;
    auto frame = (AVFrame*)e->data;
    r->delay = 0;
    e->response = &r->delay;

    if( r->env && r->audio_track_render){
        JavaVM *vm = r->vm;
        JNIEnv *cur_thread_env = nullptr;
        vm->AttachCurrentThread(&cur_thread_env, nullptr);
        jshortArray data = cur_thread_env->NewShortArray( frame->linesize[0]/2);
        short *tmp = (short*)malloc(frame->linesize[0]/2);
        for (int i = 0; i < frame->linesize[0]/2; ++i) {
            int loc = i*2;
            short val = frame->data[0][loc];
            val +=((short)(frame->data[0][loc+1])) << 8;
            tmp[i] = val;
        }
        cur_thread_env->SetShortArrayRegion(data, 0, frame->linesize[0]/2, tmp);
        free(tmp);
        jint v = cur_thread_env->CallIntMethod(r->audio_track_render,
                                                  r->audio_track_render_on_update_mid,
                                                  data, frame->channels, frame->format, frame->sample_rate);
        vm->DetachCurrentThread();
        if(v != 0){
            return  LVP_E_NO_MEM;
        }
    }
    return LVP_OK;
}

static int audio_track_render_module_init(LVPModule* module, LVPMap* options, LVPEventControl *ctl, LVPLog *log){
    auto r = (LVPAudioTrackRender*)module->private_data;
    r->ctl = ctl;
    r->log = lvp_log_alloc(module->name);
    r->log->log_call = log->log_call;
    r->log->usr_data = log->usr_data;

    if(!r->env){
        lvp_error(r->log, "need jni env", NULL);
        return LVP_E_FATAL_ERROR;
    }

    r->env->GetJavaVM(&r->vm);
    r->audio_track_render = nullptr;
    jclass cls = r->env->FindClass("com/lvp/core/AudioTrackRender");
    jmethodID constructor = r->env->GetMethodID(cls, "<init>", "()V");
    r->audio_track_render_on_update_mid = r->env->GetMethodID(cls, "onAudioDataUpdate",
                                                                 "([SIII)I");
    r->audio_track_render = r->env->NewObject(cls, constructor);
    r->audio_track_render = r->env->NewGlobalRef(r->audio_track_render);

    int ret = lvp_event_control_add_listener(r->ctl, LVP_EVENT_UPDATE_AUDIO, audio_track_handle_update_audio, r);
    if(ret != LVP_OK){
        lvp_error(r->log, "add update audio listener error", NULL);
        return  LVP_E_FATAL_ERROR;
    }

    return LVP_OK;
}

static void audio_track_render_module_close(LVPModule* module){
    auto r = (LVPAudioTrackRender*)module->private_data;
}

extern "C" JNIEXPORT jlong JNICALL
Java_com_lvp_core_LvpNative_createCore(
        JNIEnv* env,
        jobject thiz
){
    lvp_core *core = lvp_core_alloc();
   // lvp_core_add_option(core, LVP_OPTIONS_AUDIO_RENDER, (void *) LVP_ANDROID_AUDIO_RENDER);
    lvp_core_add_option(core, LVP_OPTIONS_AUDIO_RENDER, (void *) LVP_ANDROID_AUDIO_TRACK_RENDER);
    lvp_core_add_option(core, LVP_OPTIONS_VIDEO_RENDER, (void *) LVP_ANDROID_VIDEO_RENDER);
    lvp_core_add_option(core, LVP_OPTIONS_JNI_ENV, env);
    lvp_core_set_custom_log(core, android_log_call, env);
    LVPAudioTrackRender* render = (LVPAudioTrackRender*)malloc(sizeof(*render));
    render->env = env;
    lvp_core_register_dynamic_module(core, audio_track_render_module_init,
                                     audio_track_render_module_close, LVP_ANDROID_AUDIO_TRACK_RENDER,
                                     LVP_MODULE_CORE|LVP_MODULE_RENDER, render);
    lvp_core_init(core);
    return (long long)core;
}

extern "C" JNIEXPORT jboolean JNICALL
Java_com_lvp_core_LvpNative_play(
        JNIEnv* env,
        jobject thiz,
        jlong core,
        jstring path
){
    auto *lvpCore = (LVPCore*)core;
    const char* url = env->GetStringUTFChars(path, 0);
    lvp_core_set_url(lvpCore, url);
    lvp_core_play(lvpCore);

    return true;
}

extern "C" JNIEXPORT void JNICALL
Java_com_lvp_core_LvpNative_surfaceDestroyed(
        JNIEnv* env,
        jobject thiz,
        jlong core,
        jobject surface)
{
    auto *lvp_core = (LVPCore*)core;
    LVPSENDEVENT(lvp_core->event_control, LVP_EVENT_AN_SURFACE_DESTROYED, surface);
}

extern "C" JNIEXPORT void JNICALL
Java_com_lvp_core_LvpNative_surfaceCreated(
        JNIEnv* env,
        jobject thiz,
        jlong core,
        jobject surface)
{
    auto *lvp_core = (LVPCore*)core;
    LVPSENDEVENT(lvp_core->event_control, LVP_EVENT_AN_SURFACE_CREATED, surface);
}

