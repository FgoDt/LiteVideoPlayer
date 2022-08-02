#ifndef _LVP_CORE_H_
#define _LVP_CORE_H_

#include "lvp_error.h"
#include "lvp_config.h"
#include "lvp_define.h"
#include "lvp_events.h"



/**
 *lvp custom lvp_mem_free use to some eglContext lvp_mem_free self
 *@param data lvp_mem_free object 
 *@param usr_data usr data
 */
typedef void(*lvp_custom_free)(void *data,void *usr_data);

/**
 * lvp custom log 
 * @param log lvp log string
 * @param usr_data call back usr data;
 */
typedef void(*lvp_custom_log)(const char *log,void *usr_data);

typedef struct lvp_log LVPLog;
typedef struct lvp_map LVPMap;
typedef struct lvp_event_control LVPEventControl;  
typedef struct lvp_list LVPList;
typedef struct lvp_module LVPModule;

#ifdef LVP_WIN
typedef CRITICAL_SECTION lvp_mutex;
#endif // LVP_WIN
#ifdef LVP_LINUX
#include <pthread.h>
typedef pthread_mutex_t lvp_mutex;
#endif // LVP_LINUX

#include "lvp_mem.h"
#include "lvp_str.h"
#include "lvp_mutex.h"
#include "lvp_thread.h"
#include "lvp_log.h"
#include "lvp_time.h"
#include "lvp_list.h"
#include "lvp_map.h"
#include "lvp_queue.h"
#include "lvp_nqueue.h"
#include "lvp_stack.h"
#include "lvp_event.h"
#include "lvp_semaphore.h"
#include "lvp_module.h"
#include "lvp_cpp.h"

typedef struct lvp_core{
    LVPList *modules; /***< all lvp module*/
    LVPMap *options; /***< user options*/
    LVPEventControl *event_control;
    LVPLog *log;
	LVPMap* extra_modules;

    int status;
    char *option_str;
    char *input_str;
}LVPCore;

typedef int (*custom_module_init)(LVPModule *module, LVPMap *options, LVPEventControl *ctl,LVPLog *log) ;

typedef void (*custom_module_close)(LVPModule *module) ;

/**
 * lvp core alloc
 * @return NULL if error
 */
LVPCore* lvp_core_alloc();

/**
 * lvp core free
 */
void lvp_core_free(LVPCore *core);

/**
 * lvp core init all module
 */
int lvp_core_init(LVPCore* core);

/**
 * lvp core set url, set net or local media path
 * @param core lvp core 
 * @param input net or local media path eg. http://localhost:8080/vod/test.mp4 
 *                                          d:/vod/test.mkv
 * @return LVP_OK for success
 */
int lvp_core_set_url(LVPCore *core, const char *input);

/**
 * lvp core set option, set lvp options
 * @param core lvp core
 * @param options lvp custom options eg. "-reader myflv_reader -vrender qt_render"
 * @return LVP_OK for success
 */
int lvp_core_set_option(LVPCore *core, const char *options);

int lvp_core_add_option(LVPCore *core, const char *key, void *val);

/**
 * lvp core set custom log
 * @param core lvp core
 * @param log usr custom log func may be void(const char *log, void *usr_data);
 */
int lvp_core_set_custom_log(LVPCore *core,lvp_custom_log log,void *usr_data);

/**
 * lvp core play media
 * @param core lvp core
 * @return LVP_OK for success
 */
int lvp_core_play(LVPCore *core);

/**
 * lvp core pause media
 * @param core lvp core
 * @return LVP_OK for success
 */
int lvp_core_pause(LVPCore *core);

/**
 *  lvp core resume media
 *  @param core lvp core
 */
int lvp_core_resume(LVPCore* core);

/**
 * lvp core stop media
 * @param core lvp core
 * @return LVP_OK for success
 */
int lvp_core_stop(LVPCore *core);

/**
 * lvp core seek time
 * @param core lvp core
 * @param pts seek time eg 1.0 second
 * @return LVP_OK for success
 */
int lvp_core_seek(LVPCore *core,double pts);


/**
 * lvp core register dynamic module
 * the module will store in core options with key "lvp_extra_module"
 * you can get you module in module init callback
 */
int lvp_core_register_dynamic_module(LVPCore *core,custom_module_init minit,custom_module_close mclose,
                                    const char *name, int type,void *private_data);
/**
 * load module on static mem
 * just need load once for each module
 */
int lvp_load_static_custom_module(custom_module_init minit, custom_module_close mclose,
	const char* name, int type, int private_data_size);

/**
 * lvp core unload dynamic module,
 */
void lvp_unload_static_custom_module();

void lvp_register_ff_call_back();

#endif
