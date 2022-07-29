#include "lvp_event.h"


LVPEvent *lvp_event_alloc(void *data, const char *name,LVP_BOOL must_handle){
    LVPEvent *ev = (LVPEvent*)lvp_mem_mallocz(sizeof(*ev));
    ev->data = data;
    ev->must_handle = must_handle;
    lvp_str_dump(name,&ev->event_name);
    return ev;
}

void lvp_event_free(LVPEvent *ev){
    if(ev->event_name){
        lvp_mem_free(ev->event_name);
    }
    lvp_mem_free(ev);
}

///
// listener
///

LVPEventListener *lvp_event_listener_alloc(){
    LVPEventListener *l = (LVPEventListener*)lvp_mem_mallocz(sizeof(*l));
    return l;
}

void lvp_event_listener_free(LVPEventListener *l){
    lvp_mem_free(l);
}

///
// handler
///
LVPEventHandler *lvp_event_handler_alloc(const char *name){
    LVPEventHandler *h = (LVPEventHandler*)lvp_mem_mallocz(sizeof(*h));
    lvp_str_dump(name,&h->event_name);
    h->listeners = lvp_list_alloc();
	lvp_mutex_create(&h->mutex);
    return h;
}
void lvp_event_handler_free(LVPEventHandler *h){
    if(h->event_name){
        lvp_mem_free(h->event_name);
    }
    if(h->listeners){
        lvp_list_free(h->listeners);
    }
	lvp_mutex_free(&h->mutex);
    lvp_mem_free(h);
}

static void lvp_event_handler_custom_free(void *h, void *usr_data){
    lvp_event_handler_free((LVPEventHandler*)h);
}

static void lvp_event_listener_custom_free(void *l, void *usr_data){
    lvp_event_listener_free((LVPEventListener*)l);
}

///
// control
///

LVPEventControl *lvp_event_control_alloc(){
    LVPEventControl *ctl = (LVPEventControl*)lvp_mem_mallocz(sizeof(*ctl));
    ctl->handlers = lvp_map_alloc();
    return ctl;
}

void lvp_event_control_free(LVPEventControl *ctl){
    if (ctl->handlers)
    {
        lvp_map_free(ctl->handlers);
    }
    lvp_mem_free(ctl);
}

int lvp_event_control_add_listener(LVPEventControl *ctl,
        const char *key, lvp_event_call call, void *usr_data){

    assert(ctl);
    assert(key);
    assert(call);
    //create listener
    LVPEventListener *l = lvp_event_listener_alloc();
    l->call = call;
    l->usr_data = usr_data;

    //find handler
    LVPEventHandler *h = lvp_map_get(ctl->handlers,key);
    //if not find create 
    if(!h){
        h = lvp_event_handler_alloc(key);
        lvp_map_add(ctl->handlers,key,h,NULL,lvp_event_handler_custom_free,LVP_TRUE);
    }
	lvp_mutex_lock(&h->mutex);
    lvp_list_add(h->listeners,l,NULL,lvp_event_listener_custom_free,LVP_TRUE);
	lvp_mutex_unlock(&h->mutex);

    return LVP_OK;
}

void lvp_event_control_remove_listener(LVPEventControl* ctl, const char* key, lvp_event_call call, void *usrdata) {
	assert(ctl);
	assert(key);
	assert(call);
	LVPEventHandler* h = lvp_map_get(ctl->handlers, key);
	if (!h) {
		return;
	}
	lvp_mutex_lock(&h->mutex);
	LVPListEntry* entry = h->listeners->entrys;
	while(entry)
	{
		LVPEventListener* l = (LVPEventListener*)entry->data;
		if (l->call == call && l->usr_data == usrdata) {
			lvp_list_remove(h->listeners, l);
			lvp_mutex_unlock(&h->mutex);
			return;
		}
		entry = entry->next;
	}
	lvp_mutex_unlock(&h->mutex);
}

int lvp_event_control_send_event(LVPEventControl *ctl, LVPEvent *ev){
    assert(ctl);
    assert(ev);
    LVPEventHandler *h = lvp_map_get(ctl->handlers,ev->event_name);
    //no handler find
    if(!h){
        if(ev->must_handle == LVP_TRUE){
            lvp_waring(NULL,"no handler for %s",ev->event_name);
        }
        return LVP_E_NO_MEDIA;
    }
	lvp_mutex_lock(&h->mutex);
    LVPListEntry *listener_entry = h->listeners->entrys;
	if (listener_entry == NULL)
	{
        if(ev->must_handle == LVP_TRUE){
            lvp_waring(NULL,"no handler for %s",ev->event_name);
        }
		lvp_mutex_unlock(&h->mutex);
		return LVP_E_NO_MEDIA;
	}
    int ret = 0;
    while (listener_entry)
    {
        LVPEventListener *l = (LVPEventListener*)listener_entry->data;
        int status = l->call(ev,l->usr_data);
        if(status != LVP_OK && strcmp(ev->event_name,LVP_EVENT_READER_SEND_FRAME) ){
           // lvp_waring(NULL,"event %s listener return status %d",ev->event_name,status);
        }
        ret |= status;
        listener_entry = listener_entry->next;
    }
	lvp_mutex_unlock(&h->mutex);
    return ret;
    
}