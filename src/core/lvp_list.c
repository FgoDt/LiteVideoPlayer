#include "lvp_list.h"

inline static LVPListEntry* lvp_list_entry_alloc(){
    LVPListEntry *entry = (LVPListEntry*)lvp_mem_mallocz(sizeof(*entry));
    return entry;
}

inline static void lvp_list_entry_free(LVPListEntry **entry_ptr){
    LVPListEntry *entry = (LVPListEntry*)*entry_ptr;
    if(!entry){
        return;
    }
   if(entry->need_free){
       if(entry->free){
           entry->free(entry->data,entry->usr_data);
       }
       else{
           lvp_mem_free(entry->data);
       }
   }
   lvp_mem_free(entry);
   entry = NULL;
}

LVPList *lvp_list_alloc(){
    LVPList *list = (LVPList*)lvp_mem_mallocz(sizeof(*list));
    return list;
}

void lvp_list_free(LVPList *list){
    assert(list);
    LVPListEntry *entry = list->entrys;
    while ( entry != NULL)
    {
        LVPListEntry *tmp = entry;
        entry = entry->next;
        lvp_list_entry_free(&tmp);
    }
    lvp_mem_free(list);
}



int lvp_list_add(LVPList * list, void * data, void * usr_data, lvp_custom_free cfree, LVP_BOOL need_free) {
    assert(list);
    assert(data);

    LVPListEntry *entry = lvp_list_entry_alloc();
    if(!entry){
        return LVP_E_NO_MEM;
    }
    entry->data = data;
    entry->free = cfree;
    entry->usr_data = usr_data;
    entry->need_free = need_free;

    LVPListEntry *last = NULL;

    for (LVPListEntry* e = list->entrys; e != NULL; e = e->next) 
    {
        last = e;
    }
    //header
    if(last == NULL){
        list->entrys = entry;
    }else{
        last ->next = entry;
    }
    list->size ++;

    return LVP_OK;
    
}

int lvp_list_remove(LVPList *list, void *data){
    assert(list);
    assert(data);
    LVPListEntry *pre = NULL;
    for (LVPListEntry *e = list->entrys;  e != NULL ; e = e->next) 
    {
        if(e->data == data){
            //header
            if(pre == NULL){
                list->entrys = e->next;
            }else{
                pre->next = e->next;
            }
            lvp_list_entry_free(&e);
            list->size --;
            return LVP_OK;
        }
        pre = e;
    }
    return LVP_E_NO_MEDIA;
    
}

int lvp_list_remove_at(LVPList *list, uint64_t index){
    assert(list);
    LVPListEntry *pre = NULL;
    for (LVPListEntry *e = list->entrys; e != NULL ; e = e->next)
    {
        if(index == 0){
            //header
            if(pre == NULL){
                list->entrys = e->next;
            }else{
                pre->next = e->next;
            }
            lvp_list_entry_free (&e);
            list->size --;
            return LVP_OK;
        }
        index --;
        pre = e;
    }
    return LVP_E_NO_MEDIA;
}
