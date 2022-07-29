#ifndef _LVP_LIST_H_
#define _LVP_LIST_H_
#include "lvp_core.h"

typedef struct lvp_list_entry{
    void                    *data; ///< data
    void                    *usr_data; ///< usr data
    int                     need_free; /// this data is need lvp_mem_free 1 for need
    lvp_custom_free         free; ///< custom lvp_mem_free func, if lvp_mem_free not NULL use this func to lvp_mem_free data
    struct lvp_list_entry   *next; 
}LVPListEntry;

typedef struct lvp_list{
    uint64_t    size;
    LVPListEntry *entrys;
}LVPList;

LVPList *lvp_list_alloc();

void lvp_list_free(LVPList *list);

int lvp_list_add(LVPList *list, void *data, void *usr_data,lvp_custom_free cfree,LVP_BOOL need_free);

int lvp_list_remove(LVPList *list, void *data);

int lvp_list_remove_at(LVPList *list, uint64_t index);

#endif