#ifndef _LVP_NQUEUE_H_
#define _LVP_NQUEUE_H_
#include "../core/lvp_core.h"

typedef struct lvp_queue_entry{
    void *data;
    void *usr_data;
    lvp_custom_free free;
    LVP_BOOL need_free;
}LVPQueueEntry;

typedef struct lvp_nqueue
{
    int size;
    int cap;
    int rpos;
    int wpos;
    LVPQueueEntry **datas;
}LVPNQueue;

LVPNQueue *lvp_nqueue_alloc(int size);

void lvp_nqueue_free(LVPNQueue *q);

int lvp_nqueue_push(LVPNQueue *q, void *data,void *usr_data, lvp_custom_free free, LVP_BOOL need_free);

void *lvp_nqueue_pop(LVPNQueue *q);

void lvp_nqueue_clear(LVPNQueue* q);

#endif // !_LVP_NQUEUE_H_
