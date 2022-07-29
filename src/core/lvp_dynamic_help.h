//typedef int (*custom_module_init)(LVPModule* module, LVPMap* options, LVPEventControl* ctl, LVPLog* log);

//typedef void (*custom_module_close)(LVPModule* module);

typedef void* (*lvp_core_alloc_d)();
typedef int (*lvp_core_set_url_d)(void* core, const char* input);
typedef int (*lvp_core_play_d)(void* core);
