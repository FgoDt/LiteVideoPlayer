#include "../testhelp.h"
#include "../../core/lvp_nqueue.h"

int main(){
    LVPNQueue *q = lvp_nqueue_alloc(100);
    for (size_t i = 0; i < 100; i++)
    {
        lvp_nqueue_push(q,i,NULL,NULL,LVP_FALSE);
    }

    int ret = lvp_nqueue_push(q,101,NULL,NULL,LVP_FALSE);
    test_cond("push queue out of size ",ret == LVP_E_NO_MEM);
    int *data = NULL ;
    for (size_t i = 0; i < 100; i++)
    {
        int *ret = lvp_nqueue_pop(q);
        test_cond("pop queue",*ret == i);
    }

    ret = lvp_nqueue_pop(q);
    test_cond("pop no data",ret == NULL);

    test_report();

    return 0;
    
}