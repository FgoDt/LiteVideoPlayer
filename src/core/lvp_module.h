#ifndef _LVP_MODULE_H_
#define _LVP_MODULE_H_

#include "lvp_core.h"

typedef enum lvp_module_type{
    LVP_MODULE_CORE = 1,
    LVP_MODULE_READER = 2,
    LVP_MODULE_PKT_FILTER = 4,
    LVP_MODULE_DECODER = 8,
    LVP_MODULE_FRAME_FILTER = 0x10,
    LVP_MODULE_RENDER = 0x20,
}LVPModuleType;


struct lvp_module{
    uint64_t        version;
    char            *name;
    LVPModuleType   type;
    int             private_data_size;
    void            *private_data;

    int             (*module_init)(struct lvp_module *module, 
                                    LVPMap *options,
                                    LVPEventControl *ctl,
                                    LVPLog *log);
    void            (*module_close)(struct lvp_module *module);

};


/**
 * get module with type and module name
 * this function create new module so you can use this return 
 * @param module_name module name, if name = NULL will return first module the type you pass
 * @param type what type you want, must pass
 * @param extra_module extra module, may user dynamic add use lvp_core_add_dynamic_module
 * @return NULL for not find
 */
LVPModule* lvp_module_get_module(const char* module_name, LVPModuleType type, LVPMap* extra_module);

/**
 * init module
 * @param module module you want init
 * @param options
 * @param ctl module event control
 * @param log
 * @return LVP_OK for success
 */
int lvp_module_init(LVPModule *module,LVPMap *options, LVPEventControl *ctl, LVPLog *log);

/**
 * iterate module , but NOT use it 
 * you MUST use lvp_module_create_module to create you module
 * eg.
 * void *opaque = NULL;
 * LVPModuel *my_module = NULL;
 * for(;(LVPModule *m = lvp_module_iterate(&opaque))!=NULL;){
 *      if(!strcmp(m->name,"MY_MODULE_NAME")){
 *          my_module = lvp_module_create_module(m);
 *      } 
 * }
 * //then use it
 * 
 */
LVPModule* lvp_module_iterate(void **opaque);

/**
 * create new module 
 * @return NULL for no memory
 */
LVPModule* lvp_module_create_module(LVPModule *m);

void lvp_module_close(LVPModule *m);

/**
 * add dynamic module
 */
int lvp_module_add(LVPModule *m);

/**
 * free dynamic module
 */
void lvp_module_free_dynamic_module();


#endif