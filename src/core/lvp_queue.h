/*#ifndef LVP_QUEUE_H_
#define LVP_QUEUE_H_

#include "lvp_core.h"

typedef struct
{
    void **datas;
    void **usrDatas;
    lvp_custom_free* freeFuncs;
    LVP_BOOL* needFrees;
    int64_t cap;
    int64_t front;
    int64_t back;
}LVPQueue;

LVPQueue* lvp_queue_create();
LVP_BOOL lvp_queue_free(LVPQueue* queue);
LVP_BOOL lvp_queue_clear(LVPQueue* queue);
LVP_BOOL lvp_queue_push(LVPQueue* queue,void *data, void *usrData, lvp_custom_free freeFunc, LVP_BOOL needFree);
LVP_BOOL lvp_queue_push_raw(LVPQueue* queue,void *data);
LVP_BOOL lvp_queue_pop(LVPQueue* queue);
LVP_BOOL lvp_queue_size(LVPQueue* queue, int64_t* size);
LVP_BOOL lvp_queue_front(LVPQueue* queue, void** data, void **usrData);
LVP_BOOL lvp_queue_back(LVPQueue* queue, void** data, void **usrData);

#endif // !LVP_QUEUE_H_
*/
