#ifndef LVP_STACK_H_
#define LVP_STACK_H_

#include "lvp_core.h"

typedef struct 
{
    void **datas;
    void **usrDatas;
    lvp_custom_free *freeFuncs;
    LVP_BOOL *needFrees;
    int64_t cap;
    int64_t top;
    int64_t bottom;
}LVPStack;

LVPStack* lvp_stack_create();
LVP_BOOL  lvp_stack_free(LVPStack* stack);
LVP_BOOL  lvp_stack_clear(LVPStack* stack);
LVP_BOOL  lvp_stack_push(LVPStack* stack, void* data, void *usrData, lvp_custom_free freeFunc, LVP_BOOL needFree);
LVP_BOOL  lvp_stack_push_raw(LVPStack* stack, void* data);
LVP_BOOL  lvp_stack_pop(LVPStack* stack);
LVP_BOOL  lvp_stack_top(LVPStack* stack, void** data, void** usrData);
LVP_BOOL  lvp_stack_size(LVPStack* stack, int64_t* size);


#endif // !LVP_STACK_H_

