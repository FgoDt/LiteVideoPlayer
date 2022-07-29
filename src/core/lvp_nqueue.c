#include "lvp_nqueue.h"

LVPNQueue *lvp_nqueue_alloc(int cap){
    LVPNQueue *q = (LVPNQueue*)lvp_mem_mallocz(sizeof(*q));
    q->cap = cap;
    q->datas = lvp_mem_mallocz(sizeof(LVPQueueEntry)*cap);
	for (size_t i = 0; i < cap; i++)
	{
		q->datas[i] = lvp_mem_mallocz(sizeof(LVPQueueEntry));
	}
    return q;
}

void lvp_nqueue_free(LVPNQueue *q){
  
	lvp_nqueue_clear(q);

    for (size_t i = 0; i < q->cap; i++)
    {
        lvp_mem_free(q->datas[i]);
    }

	lvp_mem_free(q->datas);
    lvp_mem_free(q);
    
}

int lvp_nqueue_push(LVPNQueue *q, void *data,void *usr_data, lvp_custom_free free,LVP_BOOL need_free){
    if(q->size==q->cap){
        return LVP_FALSE;
    }
    LVPQueueEntry *entry = q->datas[q->wpos];
    entry->data = data;
    entry->usr_data = usr_data;
    entry->need_free = need_free;
    entry->free = free;
    q->wpos++;
    q->size++;
	if (q->wpos == q->cap) {
		q->wpos = 0;
	}
    return LVP_TRUE;
}

void* lvp_nqueue_pop(LVPNQueue *q){
    if(q->size==0){
        return NULL;
    }
    LVPQueueEntry *entry = q->datas[q->rpos];
    q->rpos++;
	if (q->rpos == q->cap) {
		q->rpos = 0;
	}
    q->size--;
    return entry->data;
}

void lvp_nqueue_clear(LVPNQueue* q)
{

	while (q->size>0)
	{
		if (q->rpos == q->cap)
		{
			q->rpos = 0;
		}

		LVPQueueEntry* entry = q->datas[q->rpos];
		if (entry->need_free == LVP_TRUE) {
			if (entry->free) {
				entry->free(entry->data, entry->usr_data);
			}
			else
			{
				lvp_mem_free(entry->data);
			}
			entry->data = NULL;
			entry->usr_data = NULL;
		}
		q->size--;
		q->rpos++;
	}
	q->rpos = 0;
	q->wpos = 0;
	q->size = 0;
	return;
}
