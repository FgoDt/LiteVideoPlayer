#include "../testhelp.h"
#include "../../core/lvp_list.h"

typedef struct  test_data{
    int i_val;
    char *str_val;
    double d_val;
}TestData;

void test_data_free(void *data, void *usr_data){
    assert(data);
    TestData *td = (TestData*)data;
    if(td->str_val){
        lvp_mem_free(td->str_val);
    }
    td->d_val = 0;
    td->i_val = 0;
    free(td);
}

int main(){

    LVPList* list = lvp_list_alloc();
    test_cond("list alloc",list != NULL && list->size == 0 &&list->entrys == NULL);

    int a = 1;
    int b = 2;
    int c = 3;
    lvp_list_add(list,&a,NULL,NULL,0);
    lvp_list_add(list,&b,NULL,NULL,0);
    lvp_list_add(list,&b,NULL,NULL,0);
    test_cond("list add",list->size == 3 && list->entrys->data == &a && list->entrys->next->data == &b 
                        && list->entrys->next->next->data == &b);
    int ret = lvp_list_remove(list,&b);
    test_cond("list remove mid", ret == LVP_OK && list->entrys->next->data == &b && list->entrys->next->next == NULL);

    ret = lvp_list_remove(list,&a);
    test_cond("list remove head",ret == LVP_OK && list->entrys->data == &b && list->entrys->next == NULL);

    ret = lvp_list_remove_at(list,0);
    test_cond("list remove at 0",ret == LVP_OK && list->entrys == NULL);

    lvp_list_add(list,&a,NULL,NULL,0);
    lvp_list_add(list,&b,NULL,NULL,0);
    lvp_list_add(list,&c,NULL,NULL,0);

    ret = lvp_list_remove_at(list,list->size-1);
    test_cond("list remove at size-1",ret == LVP_OK && list->size == 2 && list->entrys->next->data ==&b 
                             && list->entrys->next->next == NULL);
    TestData *td = lvp_mem_mallocz(sizeof(*td));
    td->i_val = 100;
    td->d_val = 200.0;
    td->str_val = lvp_mem_mallocz(sizeof("300")+1);
    memcpy(td->str_val,"300",3);
    lvp_list_add(list,td,td,NULL,LVP_FALSE);
    test_cond("list add struct",ret == LVP_OK );

    lvp_list_free(list);
    test_cond("list free without data free",td->i_val == 100 && td->d_val == 200.0);

    list = lvp_list_alloc();
    lvp_list_add(list,td,NULL,test_data_free,1);
    lvp_list_free(list);

    test_report();
}