#include "../testhelp.h"
#include "../../core/lvp_str.h"

int main(){
    const char *src = "abcd123";
    const char *src2 = "abcd";

    char *dst = NULL;
    int ret =  lvp_str_dump(src,&dst);
    test_cond("lvp dump str ",ret == LVP_OK && dst != NULL &&!strcmp(dst,src));
    lvp_mem_free(dst);
    dst = NULL;
    ret = lvp_str_dump_with_size(src,&dst,10);
    test_cond("lvp dump str with size 10",ret != LVP_OK && dst == NULL);

    ret = lvp_str_dump_with_size(src,&dst,strlen(src2)); 
    test_cond("lvp dum str with size sizeof(src2)",ret == LVP_OK && dst != NULL && !strcmp(dst,src2));
    lvp_mem_free(dst);

    LVPMap *option = lvp_map_alloc();
    ret = lvp_str_parse_to_options("abc",option);
    test_cond("lvp str parse error string",ret == LVP_OK);

    ret = lvp_str_parse_to_options("-r abc -d def -f fc",option);
    test_cond("lvp str parse",ret==LVP_OK);
    char *key = (char*)lvp_map_get(option,"r");
    test_cond("get option key",!strcmp(key,"abc"));
    key = (char*)lvp_map_get(option,"d");
    test_cond("get option key",!strcmp(key,"def"));
    key = (char*)lvp_map_get(option,"f");
    test_cond("get option key",!strcmp(key,"fc"));

    lvp_map_free(option);


    test_report();

    return 0;
}