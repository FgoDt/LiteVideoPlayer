#include "../../core/lvp_event.h"
#include "../testhelp.h"

static int call_count = 0;
static int test_abc_call(LVPEvent *ev, void *usr_data){
    test_cond("abc call",ev!=NULL&&!strcmp(ev->data,usr_data)&&!strcmp(ev->data,"abc"));
    call_count++;
    return 0;
}

static int test_other_abc(LVPEvent *ev, void *usr_data){
    test_cond("same abc handler but return error call",ev!=NULL&&!strcmp(ev->data,"abc"));
    call_count++;
    return -1;
}


static int test_return_error(LVPEvent *ev,void *usr_data){
    test_cond("error handler call",ev!=NULL&&usr_data==NULL);
    call_count++;
    return -11;
}

int main(){
    LVPEventControl *ctl = lvp_event_control_alloc();
    test_cond("event control alloc",ctl!=NULL);

    const char *data = "abc";
    LVPEvent *ev = lvp_event_alloc((void*)data,"abc");
    test_cond("event alloc", ev != NULL);

    int ret = lvp_event_control_add_listener(ctl,"abc",test_abc_call,"abc");
    test_cond("add listener",ret==LVP_OK);

    ret = lvp_event_control_add_listener(ctl,"error",test_return_error,NULL);
    test_cond("add error listener ",ret==LVP_OK);

    ret = lvp_event_control_send_event(ctl,ev);
    test_cond("send ev abc ",ret == LVP_OK);

    LVPEvent *error_ev= lvp_event_alloc((void*)data,"error");
    ret = lvp_event_control_send_event(ctl,error_ev);
    test_cond("send error ev", ret != LVP_OK);

    ret = lvp_event_control_add_listener(ctl,"abc",test_other_abc,NULL);
    test_cond("add other abc", ret == LVP_OK);

    ret = lvp_event_control_send_event(ctl,ev);
    test_cond("send ev ",ret!=LVP_OK);

    ret = lvp_event_control_add_listener(ctl,"error",test_abc_call,"abc");
    test_cond("add listener to error",ret == LVP_OK);

    call_count = 0;
    lvp_event_control_send_event(ctl,error_ev);
    test_cond("call count",call_count == 2);

    lvp_event_free(ev);
    lvp_event_free(error_ev);

    lvp_event_control_free(ctl);

    test_report();

}