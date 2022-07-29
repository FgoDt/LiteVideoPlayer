//
// Created by fgodt on 2022/7/26.
//

#include "lvp.h"
#include "lvp_module.h"
#include "lvp_mutex.h"
#include "lvp_thread.h"
#include "lvp_time.h"

typedef struct LVPProfile{
    LVPLog *log;
    LVPEventControl  *ctl;
    lvp_mutex mutex;
    lvp_thread collection_thread;

    int exit;
}LVPProfile;

#define LVP_EVENT_COLLECTION_DATA(p, type, c) {\
c = 0;\
LVPEvent *e = lvp_event_alloc(NULL, type, LVP_FALSE);\
int ret = lvp_event_control_send_event(p->ctl, e);\
if (ret == LVP_OK) {\
c = *(long *) e->response;\
}\
lvp_event_free(e);\
}

static void* profile_collection_thread(void* data){
    LVPProfile *p = data;
    p->exit = 0;
    while (!p->exit){
        //FIXME use cur time
        lvp_sleep(900);

        continue;

        long collection = 0;
        LVP_EVENT_COLLECTION_DATA(p, LVP_EVENT_PROFILE_READER_AUDIO_FPS,collection)
        lvp_debug(p->log, "%s %ld",LVP_EVENT_PROFILE_READER_AUDIO_FPS, collection);

        LVP_EVENT_COLLECTION_DATA(p, LVP_EVENT_PROFILE_READER_VIDEO_FPS,collection);
        lvp_debug(p->log, "%s %ld",LVP_EVENT_PROFILE_READER_VIDEO_FPS, collection);

        LVP_EVENT_COLLECTION_DATA(p, LVP_EVENT_PROFILE_CACHE_AUDIO_PKT,collection);
        lvp_debug(p->log, "%s %ld",LVP_EVENT_PROFILE_CACHE_AUDIO_PKT, collection);

        LVP_EVENT_COLLECTION_DATA(p, LVP_EVENT_PROFILE_CACHE_AUDIO_FRAME,collection);
        lvp_debug(p->log, "%s %ld",LVP_EVENT_PROFILE_CACHE_AUDIO_FRAME, collection);

        LVP_EVENT_COLLECTION_DATA(p, LVP_EVENT_PROFILE_CACHE_VIDEO_PKT,collection);
        lvp_debug(p->log, "%s %ld",LVP_EVENT_PROFILE_CACHE_VIDEO_PKT, collection);

        LVP_EVENT_COLLECTION_DATA(p, LVP_EVENT_PROFILE_CACHE_VIDEO_FRAME,collection);
        lvp_debug(p->log, "%s %ld",LVP_EVENT_PROFILE_CACHE_VIDEO_FRAME, collection);
    }
}

static int module_init(struct lvp_module *module,
        LVPMap *option, LVPEventControl *ctl, LVPLog *log){
    assert(module);
    assert(ctl);
    assert(log);

    LVPProfile *p = (LVPProfile*)module->private_data;
    p->ctl = ctl;
    p->log = lvp_log_alloc(module->name);
    p->log->log_call = log->log_call;
    p->log->usr_data = log->usr_data;

    LVP_BOOL ret =lvp_mutex_create(&p->mutex);
    if(ret != LVP_TRUE){
        lvp_error(p->log, "create mutex error", NULL);
        return  ret;
    }

    ret = lvp_thread_create(&p->collection_thread, profile_collection_thread, p);
    if(ret != LVP_TRUE){
        lvp_error(p->log, "create collection thread failed", NULL);
        return ret;
    }
    return LVP_OK;

}

static void module_close(struct lvp_module *m){

}

LVPModule lvp_profile_module = {
        .version = lvp_version,
        .name = "LVP_PROFILE_MODULE",
        .type = LVP_MODULE_CORE,
        .private_data_size = sizeof(LVPProfile),
        .private_data = NULL,
        .module_init = module_init,
        .module_close = module_close,
};

