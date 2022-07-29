#include "../../core/lvp_log.h"
#include "../testhelp.h"

void test_log(const char *log, void *usr_data){
    test_cond("call back test",log!=NULL);
}

int main(){
    LVPLog* log = lvp_log_alloc(NULL);
    test_cond("log alloc",log!=NULL && log->ctx_name == NULL);
    lvp_log_free(log);
    log = lvp_log_alloc("abc");
    log->usr_data = "abc";
    log->log_call = test_log;
    test_cond("log alloc with name",log!=NULL && !strcmp(log->ctx_name,"abc"));
    lvp_error(log,"log test ",NULL);
    lvp_waring(log,"log waring %s","hello str");
    lvp_debug(log,"log debug %s %d %f","hello str",100,200.0);
    lvp_log_free(log);
    test_report();
    return 0;
}