#include "lvp_module.h"
#include "lvp_modules.h"

LVPModule* lvp_module_get_module(const char *module_name, LVPModuleType type, LVPMap *extra_module){
    LVPModule *m = NULL;
    void *op = NULL;
    for ( ; (m = lvp_module_iterate(&op))!= NULL;){
        if(type == m->type){
            if(module_name && !strcmp(module_name, m->name)){
                break;
            }else{
                if(module_name){
                    m = NULL;
                    continue;
                }else{
                    break;
                }
            }
            break;
        }
        m = NULL;
    }
    if(!m){
		m = lvp_map_get(extra_module, module_name);
        return m;
    }
    LVPModule *ret = (LVPModule*)lvp_mem_mallocz(sizeof(*ret));
    memcpy(ret,m,sizeof(*ret));
    return ret;
}

int lvp_module_init(LVPModule *module,LVPMap *options, LVPEventControl *ctl, LVPLog *log){
    assert(module);
    assert(options);
    assert(ctl);
    if(!module->module_init){
        lvp_error(NULL,"module %s no function for init ",module->name);
        return LVP_E_FATAL_ERROR;
    }
    //dynamic module can use own data when register to module list
    if(!module->private_data) {
        module->private_data = lvp_mem_mallocz(module->private_data_size);
    }
    int ret = module->module_init(module,options,ctl,log);
    if(ret != LVP_OK)
        lvp_waring(NULL,"module %s init return %d",module->name,ret);
    else
        lvp_debug(NULL,"module %s init return %d",module->name,ret);

    return ret;
}


LVPModule* lvp_module_iterate(void **opaque){
    size_t size = sizeof(LVPModules)/sizeof(LVPModules[0])-1;
    uintptr_t i = (uintptr_t)*opaque;
    LVPModule *m = NULL;
    if(i<=size){
        m = LVPModules[i];
    }
    if(DynamicModuleNum >0 && i-size < DynamicModuleNum){
        m = DynamicModules[i-size];
    }
    if(m){
        *opaque = (void*)(i+1);
    }
    return m;
}

LVPModule* lvp_module_create_module(LVPModule *m){
    LVPModule *ret = (LVPModule*)lvp_mem_mallocz(sizeof(*ret));
    memcpy(ret,m,sizeof(*m));
    return ret;
}

void lvp_module_close(LVPModule *m){
    m->module_close(m);
}

int lvp_module_add(LVPModule *m){
    DynamicModuleNum++;
    if(DynamicModules == NULL){
        DynamicModules = lvp_mem_mallocz(sizeof(LVPModule));
        DynamicModules[0]  = m;
        return LVP_OK;
    }
    DynamicModules = lvp_mem_realloc(DynamicModules,DynamicModuleNum*sizeof(LVPModule));
    DynamicModules[DynamicModuleNum-1] = m;
    return LVP_OK;
}

void lvp_module_free_dynamic_module(){
    DynamicModuleNum = 0;
    lvp_mem_free(DynamicModules);
    DynamicModules = NULL;
}