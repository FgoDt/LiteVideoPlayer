#include "lvp_pkt_filter.h"
#include <libavcodec/avcodec.h>

static int handle_reader_send_frame(LVPEvent *ev, void *usr_data){
    assert(ev);
    LVPPktFilter *f = (LVPPktFilter*)usr_data;

	//resend filted pkt
	if (f->filtered_pkt != NULL) {
		LVPEvent *must_handle_ev = lvp_event_alloc(f->filtered_pkt,LVP_EVENT_FILTER_SEND_PKT,LVP_TRUE);
		int ret = lvp_event_control_send_event(f->ctl,must_handle_ev);
		if (ret == LVP_E_NO_MEM) {
			lvp_event_free(must_handle_ev);
			return ret;
		}
		if (ret == LVP_OK){
			av_packet_free(&f->filtered_pkt->pkt);
            lvp_pkt_free(f->filtered_pkt);
			f->filtered_pkt = NULL;
			lvp_event_free(must_handle_ev);
        }
        else {
            return ret;
        }
	}

    LVPPkt* lpkt = ev->data;
	
    LVPPkt* src_lpkt = lvp_pkt_clone(lpkt);

	// src_pkt is ref data if sub module change ev->data MUST FREE src_pkt
    //for sub module use
	LVPEvent* sub_event = lvp_event_alloc(src_lpkt, LVP_EVENT_FILTER_GOT_PKT, LVP_FALSE);
	lvp_event_control_send_event(f->ctl, sub_event);

    //for other core module use
    LVPEvent *must_handle_ev = lvp_event_alloc(sub_event->data,LVP_EVENT_FILTER_SEND_PKT,LVP_TRUE);
    int ret = lvp_event_control_send_event(f->ctl,must_handle_ev);

	//this packet need resend
	if (ret == LVP_E_NO_MEM) {
        f->filtered_pkt = src_lpkt;
        //we handle this packet so for reader this ret should be LVP_OK
        ret = LVP_OK;
	}
	else {
		src_lpkt = sub_event->data;
		av_packet_free(&src_lpkt->pkt);
        lvp_pkt_free(src_lpkt);
	}

	lvp_event_free(must_handle_ev);
	lvp_event_free(sub_event);
    
    return ret;
}

static void filter_sub_module_free(void *data,void *usr_data){
    LVPModule *m = (LVPModule*)data;
    lvp_module_close(m);
    lvp_mem_free(m);
}

static int filter_init(struct lvp_module *module, 
                                    LVPMap *options,
                                    LVPEventControl *ctl,
                                    LVPLog *log){
    assert(module);
    assert(module->private_data);
    assert(ctl);
    LVPPktFilter *filter = (LVPPktFilter*)module->private_data;
    filter->log = lvp_log_alloc(module->name);
    filter->log->log_call = log->log_call;
    filter->log->usr_data = log->usr_data;

    filter->ctl = ctl;

    int ret = lvp_event_control_add_listener(filter->ctl,LVP_EVENT_READER_SEND_FRAME,handle_reader_send_frame,filter);
    if(ret != LVP_OK){
        lvp_error(filter->log,"add listener for %s error return %d",LVP_EVENT_READER_SEND_FRAME,ret);
        return ret;
    }

    //init sub module
    filter->modules = lvp_list_alloc();
    void *op = NULL;
    for (LVPModule *m = NULL; (m = lvp_module_iterate(&op)) != NULL; )
    {
        if(m->type == LVP_MODULE_PKT_FILTER){
            LVPModule *c = lvp_module_create_module(m);
            if(c!=NULL){
                int ret = lvp_module_init(c,options,ctl,log);
                if(ret != LVP_OK){
                    lvp_module_close(c);
                    lvp_mem_free(c);
                }
                lvp_list_add(filter->modules,c,NULL,filter_sub_module_free,1);
            }
        }
    }
    


    return LVP_OK;
}

static void filter_close(struct lvp_module *module){
    assert(module);
    LVPPktFilter *f = (LVPPktFilter*)module->private_data;
    if(f->modules){
        lvp_list_free(f->modules);
    }
	if (f->log) {
		lvp_log_free(f->log);
	}
	if (f->filtered_pkt) {
		av_packet_free(&f->filtered_pkt->pkt);
		lvp_pkt_free(f->filtered_pkt);
	}
	lvp_mem_free(f);
	module->private_data = NULL;
}


LVPModule lvp_pkt_filter = {
    .version = lvp_version,
    .name = "LVP_PKT_FILTER",
    .type = LVP_MODULE_CORE|LVP_MODULE_PKT_FILTER,
    .private_data_size = sizeof(LVPPktFilter),
    .private_data = NULL,
    .module_init = filter_init,
    .module_close = filter_close,
};