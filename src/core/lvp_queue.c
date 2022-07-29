/*#include "lvp_queue.h"
#include <stdlib.h>

void lvp_queue_add_cap(LVPQueue* queue)
{
#define Queue_Default_Cap 8

    int64_t newCap = 0;
    if (queue->cap< Queue_Default_Cap)
    {
        newCap = Queue_Default_Cap;
    }
    else
    {
        newCap = queue->cap * 2;
    }

    void **datas = (void**)lvp_mem_malloc(sizeof(void*)*newCap);
    void **usrDatas = (void**)lvp_mem_malloc(sizeof(void*)*newCap);
    lvp_custom_free *freeFuncs = (lvp_custom_free*)lvp_mem_malloc(sizeof(lvp_custom_free)*newCap);
    LVP_BOOL *needFrees = (LVP_BOOL*)lvp_mem_malloc(sizeof(LVP_BOOL)*newCap);

    for (int64_t i = queue->front; i < queue->back; i++)
    {
        int64_t j = i - queue->front;
        datas[j] = queue->datas[i];
        usrDatas[j] = queue->usrDatas[j];
        freeFuncs[j] = queue->freeFuncs[j];
        needFrees[j] = queue->needFrees[j];
    }

    lvp_mem_free(queue->datas);
    lvp_mem_free(queue->usrDatas);
    lvp_mem_free(queue->freeFuncs);
    lvp_mem_free(queue->needFrees);

    queue->datas = datas;
    queue->usrDatas = usrDatas;
    queue->freeFuncs = freeFuncs;
    queue->needFrees = needFrees;

    queue->back = queue->back - queue->front;
    queue->front = 0;
    queue->cap = newCap;
}

void lvp_queue_move_data(LVPQueue* queue)
{
    for (int64_t i = queue->front; i < queue->back; i++)
    {
        int64_t j = i - queue->front;
        queue->datas[j] = queue->datas[i];
        queue->usrDatas[j] = queue->usrDatas[i];
        queue->freeFuncs[j] = queue->freeFuncs[i];
        queue->needFrees[j] = queue->needFrees[i];
    }

    queue->back = queue->back - queue->front;
    queue->front = 0;
}

LVPQueue * lvp_queue_create()
{
    LVPQueue *queue = (LVPQueue*)lvp_mem_malloc(sizeof(LVPQueue));

    queue->datas = LVP_LP_NULL;
    queue->usrDatas = LVP_LP_NULL;
    queue->freeFuncs = LVP_NULL;
    queue->needFrees = LVP_NULL;

    queue->cap = 0;
    queue->front = 0;
    queue->back = 0;

    lvp_queue_add_cap(queue);

    return queue;
}
LVP_BOOL lvp_queue_free(LVPQueue * queue)
{
    if (LVP_NULL == queue)
    {
        return LVP_FALSE;
    }

    for (int64_t i = queue->front; i < queue->back; i++)
    {
        lvp_custom_free freeFunc = queue->freeFuncs[i];
        LVP_BOOL needFree = queue->needFrees[i];
        if (needFree)
        {
            if (freeFunc != LVP_NULL)
            {
                freeFunc(queue->datas[i], queue->usrDatas[i]);
            }
            else
            {
                lvp_mem_free(queue->datas[i]);
            }
        }
    }

    lvp_mem_free(queue->datas);
    lvp_mem_free(queue->usrDatas);
    lvp_mem_free(queue->freeFuncs);
    lvp_mem_free(queue->needFrees);
    lvp_mem_free(queue);

    return LVP_TRUE;
}

LVP_BOOL lvp_queue_clear(LVPQueue * queue)
{
    if (LVP_NULL == queue)
    {
        return LVP_FALSE;
    }

    for (int64_t i = queue->front; i < queue->back; i++)
    {
        lvp_custom_free freeFunc = queue->freeFuncs[i];
        LVP_BOOL needFree = queue->needFrees[i];
        if (needFree)
        {
            if (freeFunc != LVP_NULL)
            {
                freeFunc(queue->datas[i], queue->usrDatas[i]);
            }
            else
            {
                lvp_mem_free(queue->datas[i]);
            }
        }
    }

    queue->front = 0;
    queue->back = 0;

    return LVP_TRUE;
}

LVP_BOOL lvp_queue_push(LVPQueue * queue, void * data, void * usrData, lvp_custom_free freeFunc, LVP_BOOL needFree)
{
    if (LVP_NULL == queue)
    {
        return LVP_FALSE;
    }

    int64_t space = queue->cap - queue->back;

    if (0 == space)
    {
        int64_t used = queue->back - queue->front;
        if (used*2<queue->cap)
        {
            lvp_queue_move_data(queue);
        }
        else
        {
            lvp_queue_add_cap(queue);
        }
    }

    int64_t i = queue->back;
    queue->datas[i] = data;
    queue->usrDatas[i] = usrData;
    queue->freeFuncs[i] = freeFunc;
    queue->needFrees[i] = needFree;
    queue->back++;

    return LVP_TRUE;
}

LVP_BOOL lvp_queue_push_raw(LVPQueue * queue, void * data)
{
    if (LVP_NULL == queue)
    {
        return LVP_FALSE;
    }

    return lvp_queue_push(queue, data, LVP_NULL, LVP_NULL, LVP_FALSE);
}

LVP_BOOL lvp_queue_pop(LVPQueue * queue)
{
    if (LVP_NULL == queue)
    {
        return LVP_FALSE;
    }

    if (queue->front == queue->back)
    {
        return LVP_FALSE;
    }

    queue->front++;

    return LVP_TRUE;
}

LVP_BOOL lvp_queue_size(LVPQueue * queue, int64_t * size)
{
    if (LVP_NULL == queue)
    {
        return LVP_FALSE;
    }

    if (LVP_NULL ==  size)
    {
        return LVP_FALSE;
    }

    *size = queue->back - queue->front;

    return LVP_TRUE;
}

LVP_BOOL lvp_queue_front(LVPQueue * queue, void ** data, void ** usrData)
{
    if (LVP_NULL == queue)
    {
        return LVP_FALSE;
    }

    if (queue->back == queue->front)
    {
        return LVP_FALSE;
    }

    if (LVP_LP_NULL!=data)
    {
        *data = queue->datas[queue->front];
    }

    if (LVP_LP_NULL != usrData)
    {
        *usrData = queue->usrDatas[queue->front];
    }

    return LVP_TRUE;
}

LVP_BOOL lvp_queue_back(LVPQueue * queue, void ** data, void ** usrData)
{
    if (LVP_NULL == queue)
    {
        return LVP_FALSE;
    }

    if (queue->back == queue->front)
    {
        return LVP_FALSE;
    }

    int64_t i = queue->back - 1;

    if (LVP_LP_NULL != data)
    {
        *data = queue->datas[i];
    }

    if (LVP_LP_NULL != usrData)
    {
        *usrData = queue->usrDatas[i];
    }

    return LVP_TRUE;
}
*/
