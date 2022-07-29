#include "lvp_core.h"

static int init_core_modules(LVPCore *core);

LVPCore* lvp_core_alloc(){
    LVPCore *core = (LVPCore*)lvp_mem_mallocz(sizeof(*core));
    core->modules = lvp_list_alloc();
    core->options = lvp_map_alloc();
	core->extra_modules = lvp_map_alloc();
    core->event_control = lvp_event_control_alloc();
    core->log = lvp_log_alloc("LVP CORE");
    return core;
}

void lvp_core_free(LVPCore *core){
    assert(core);
    if(core->modules){
        lvp_list_free(core->modules);
    }
    if(core->options){
        lvp_map_free(core->options);
    }
    if(core->event_control){
        lvp_event_control_free(core->event_control);
    }
    if(core->log){
        lvp_log_free(core->log);
    }
    if(core->input_str){
        lvp_mem_free(core->input_str);
    }
    if(core->option_str){
        lvp_mem_free(core->option_str);
    }
	lvp_mem_free(core);
}

int lvp_core_init(LVPCore* core)
{
    int ret = init_core_modules(core);
    if(ret!=LVP_OK){
        lvp_error(core->log,"init core modules error",NULL);
        return ret;
    }
    return ret;
}

int lvp_core_set_url(LVPCore *core, const char *input){
    assert(core);
    assert(input);
    int ret = lvp_str_dump(input,&core->input_str);
    lvp_debug(core->log,"set url: %s",input);
    return ret;
}

int lvp_core_set_option(LVPCore *core, const char *options){
    assert(core);
    assert(options);
    int ret = lvp_str_dump(options,&core->option_str);
    if(ret!=LVP_OK){
        return ret;
    }
    ret = lvp_str_parse_to_options(options,core->options);
    return ret;
}

int lvp_core_add_option(LVPCore *core, const char *key, void *val){
    assert(core);
    assert(key);
    int ret = lvp_map_add(core->options, key, val, NULL, NULL, LVP_FALSE);
    return ret;
}

int lvp_core_set_custom_log(LVPCore *core,lvp_custom_log log,void *usr_data){
    assert(core);
    core->log->usr_data = usr_data;
    core->log->log_call = log;
    return LVP_OK;
}

static void lvp_module_custom_free(void* data, void* usr_data) {
	LVPModule* m = (LVPModule*)data;
	m->module_close(m);
	lvp_mem_free(m);
}

static int init_basic_module(LVPCore *core,const char *default_module_name,
                             const char *option_key, LVPModuleType type){
    LVPModule *module = NULL;
    const char *module_name = NULL;
    if(option_key != NULL){
        module_name = lvp_map_get(core->options,option_key);
    }
    module_name = module_name == NULL ? default_module_name : module_name;
    module = lvp_module_get_module(module_name,type,core->extra_modules);
    if(!module){
        lvp_error(NULL,"can not find module %s for type %d",module_name?module_name:default_module_name,type);
        return LVP_E_NO_MEDIA;
    }
    int ret = lvp_module_init(module,core->options,core->event_control,core->log);
    if(ret != LVP_OK){
        lvp_mem_free(module);
        return ret;
    }
    //所有module只负责释放结构体，module自己应该遵循习惯通过close自己释放private data 
    lvp_list_add(core->modules,module,NULL,lvp_module_custom_free,1);
    return LVP_OK;
}


static int init_core_modules(LVPCore *core){
    assert(core);

    void *op = NULL;
    for ( LVPModule *m = NULL ; (m = lvp_module_iterate(&op)) != NULL; )
    {
        if (m->type == LVP_MODULE_CORE){
            LVPModule *core_module = lvp_module_create_module(m);
            if(core_module == NULL){
                lvp_error(core->log,"create module: %s error no mem",m->name);
                return LVP_E_NO_MEM;
            }
            int ret = lvp_module_init(core_module,core->options,core->event_control,core->log);
            if(ret != LVP_OK){
                lvp_error(core->log,"init module: %s error return %d",m->name,ret);
                return ret;
            }
            lvp_list_add(core->modules,core_module,NULL,lvp_module_custom_free,1);
        }
    }

    lvp_debug(core->log,"init core module done",NULL);
    

    //reader
    int ret = init_basic_module(core,"LVP_READER_MODULE","reader",LVP_MODULE_CORE|LVP_MODULE_READER);
    if(ret != LVP_OK){
        goto error;
    }

    //pktfilter 
    ret = init_basic_module(core,"LVP_PKT_FILTER","pkt_filter",LVP_MODULE_CORE|LVP_MODULE_PKT_FILTER);
    if(ret!=LVP_OK){
        goto error;
    }

    ret = init_basic_module(core,"LVP_FRAME_FILTER","frame_filter",LVP_MODULE_CORE|LVP_MODULE_FRAME_FILTER);

    if (ret != LVP_OK){
        goto error;
    }


    ret = init_basic_module(core,"LVP_AUDIO_DECODER","audio_decoder",LVP_MODULE_CORE|LVP_MODULE_DECODER);
    if (ret != LVP_OK){
        goto error;
    }

    ret = init_basic_module(core,"LVP_VIDEO_DECODER","video_decoder",LVP_MODULE_CORE|LVP_MODULE_DECODER);
    if(ret != LVP_OK){
        goto error;
    }

    ret = init_basic_module(core,"LVP_SUB_DECODER","sub_decoder",LVP_MODULE_CORE|LVP_MODULE_DECODER);
    if(ret != LVP_OK){
        goto error;
    }

    ret = init_basic_module(core,"LVP_AUDIO_RENDER","audio_render",LVP_MODULE_CORE|LVP_MODULE_RENDER);
    if(ret != LVP_OK){
        goto error;
    }

    ret = init_basic_module(core,"LVP_VIDEO_RENDER","video_render",LVP_MODULE_CORE|LVP_MODULE_RENDER);
    if(ret != LVP_OK){
        goto error;
    }


    return LVP_OK;

    error:
        return ret;
}

int lvp_core_play(LVPCore *core){
    assert(core);
    LVPSENDEVENT(core->event_control,LVP_EVENT_SET_URL,core->input_str);
    LVPEvent *ev = lvp_event_alloc(NULL,LVP_EVENT_PLAY,LVP_TRUE);
    int ret = lvp_event_control_send_event(core->event_control,ev);
    lvp_event_free(ev);
    return ret;
    
}
int lvp_core_pause(LVPCore *core){
    assert(core);
    LVPSENDEVENT(core->event_control,LVP_EVENT_PAUSE,NULL);
    return LVP_OK;
}

int lvp_core_resume(LVPCore *core){
    assert(core);
    LVPSENDEVENT(core->event_control,LVP_EVENT_RESUME,NULL);
    return LVP_OK;
}

int lvp_core_stop(LVPCore *core){
    assert(core);
    LVPSENDEVENT(core->event_control,LVP_EVENT_STOP,NULL);
    return LVP_OK;
}

int lvp_core_seek(LVPCore *core,double pts){
    assert(core);
    //make pts is micro second
    int64_t core_pts = pts*1000*1000;
    LVPEvent *ev = lvp_event_alloc(&core_pts,LVP_EVENT_SEEK,LVP_TRUE);
    int ret = lvp_event_control_send_event(core->event_control,ev);
    lvp_event_free(ev);
    return ret;
}

int lvp_core_register_dynamic_module(LVPCore *core, custom_module_init minit,custom_module_close mclose,
                                    const char *name, int type,void *private_data){
    LVPModule *m = (LVPModule*)lvp_mem_mallocz(sizeof(LVPModule));
    m->module_init = minit;
    m->module_close = mclose;
    m->name = (char*)name;
    m->type = type;
	m->private_data = private_data;
	lvp_map_add(core->extra_modules, name, m, NULL, NULL, 1);
    lvp_map_add(core->options,"lvp_extra_module",core->extra_modules,NULL,(lvp_custom_free)lvp_map_free,1);
	return LVP_OK;
}

int lvp_load_static_custom_module(custom_module_init minit, custom_module_close mclose,
	const char* name, int type, int private_data_size) {
    LVPModule *m = (LVPModule*)lvp_mem_mallocz(sizeof(LVPModule));
    m->module_init = minit;
    m->module_close = mclose;
    m->name = (char*)name;
    m->type = type;
	m->private_data_size = private_data_size;
	return lvp_module_add(m);
}


void lvp_unload_custom_module(){
    lvp_module_free_dynamic_module();
}