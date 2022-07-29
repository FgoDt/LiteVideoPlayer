#include "../testhelp.h"
#include "../../core/lvp_map.h"

int main(){
    LVPMap *m = lvp_map_alloc();
    int ret = lvp_map_add(m,"abc","ddd",NULL,NULL,LVP_FALSE);
    test_cond("lvp map add ", ret == LVP_OK&& m->size == 1);
    ret = lvp_map_add(m,"bca","eee",NULL,NULL,LVP_FALSE);
    test_cond("lvp map add ", ret == LVP_OK);
    ret = lvp_map_add(m,"cab","fff",NULL,NULL,LVP_FALSE);
    test_cond("lvp map add ", ret == LVP_OK);

    const char *abc = lvp_map_get(m,"abc");
    test_cond("lvp map get ", abc != NULL && !strcmp(abc,"ddd"));

    const char *bca = lvp_map_get(m,"bca");
    test_cond("lvp map get ", bca != NULL && !strcmp(bca,"eee"));

    const char *cab = lvp_map_get(m,"cab");
    test_cond("lvp map get ", cab != NULL && !strcmp(cab,"fff"));

    ret = lvp_map_add(m,"abc","aaa",NULL,NULL,LVP_FALSE);
    abc = lvp_map_get(m,"abc");
    test_cond("lvp map replace",ret == LVP_OK&&!strcmp(abc,"aaa"));

    ret = lvp_map_del(m,"jkl");
    test_cond("lvp map del null key", ret == LVP_E_NO_MEDIA);

    ret = lvp_map_del(m,"abc");
    abc = NULL;
    abc = lvp_map_get(m,"abc");
    test_cond("lvp map del key", ret == LVP_OK && abc == NULL);
    
    lvp_map_free(m);

    test_report();

return 0;
}