#ifndef _LVP_EVENT_H_
#define _LVP_EVENT_H_

#include "lvp_core.h"

#define LVPSENDEVENT(ctl,event,data)            \
LVPEvent *e = lvp_event_alloc((data),(event),LVP_FALSE);  \
lvp_event_control_send_event((ctl),e);         \
lvp_event_free(e)

typedef struct lvp_event{
    char *event_name;
    void *data;
	void* response;
    int must_handle;/// < must have handler
}LVPEvent;

typedef int(*lvp_event_call)(LVPEvent *ev, void *usr_data);

typedef struct lvp_event_listener{
    lvp_event_call call;
    void *usr_data;
}LVPEventListener;

typedef struct lvp_event_handler{
    char *event_name;
    LVPList *listeners;
	lvp_mutex mutex;
}LVPEventHandler;

struct lvp_event_control{
    LVPMap *handlers;
};


///
// event 
///

//LVPEvent *lvp_event_alloc(void *data, const char *name);
LVPEvent *lvp_event_alloc(void *data, const char *name,LVP_BOOL must_handle);

void lvp_event_free(LVPEvent *ev);

///
// listener
///

LVPEventListener *lvp_event_listener_alloc();

void lvp_event_listener_free(LVPEventListener *l);

///
// handler
///
LVPEventHandler *lvp_event_handler_alloc(const char *name);
void lvp_event_handler_free(LVPEventHandler *h);

///
// control
///

LVPEventControl *lvp_event_control_alloc();

void lvp_event_control_free(LVPEventControl *ctl);

int lvp_event_control_add_listener(LVPEventControl *ctl,
        const char *key, lvp_event_call call, void *usr_data);

void lvp_event_control_remove_listener(LVPEventControl* ctl, const char* key,
		lvp_event_call call, void* usrdata);

int lvp_event_control_send_event(LVPEventControl *ctl, LVPEvent *ev);


#endif