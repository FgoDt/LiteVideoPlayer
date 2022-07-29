#include "lvp_str.h"

int lvp_str_dump_with_size(const char *src, char **dst,size_t size){
    *dst = (char*)lvp_mem_mallocz(size+1);
    if(*dst == NULL){
        return LVP_E_NO_MEM;
    }
    if(strlen(src) < size){
        lvp_mem_free(*dst);
        *dst = NULL;
        return LVP_E_NO_MEDIA;
    }
    char *tmp = (char*)*dst;
    memcpy(tmp,src,size);
    return LVP_OK;
}

int lvp_str_dump(const char *src, char **dst){
    int len = strlen(src);
    return lvp_str_dump_with_size(src,dst,len);
}

int lvp_str_parse_to_options(const char *option_str, LVPMap *options){
        assert(options);
    assert(option_str);

    char *str = (char*)option_str;
    char *option_start = 0;
    char *option_end = 0;
    char *key = NULL;
    char *val = NULL;
    int is_key = 0;
    while (str[0]!=0)
    {
        if(str[0] == '-'){
            option_start = str+1;
            is_key = 1;
        }
        if(str[0] == ' '){
            option_end = str;
            if(option_end > option_start){
                int len = option_end - option_start;
                if(is_key){
                    if(key != NULL){
                        lvp_mem_free(key);
                        key = NULL;
                    }
                    lvp_str_dump_with_size(option_start,&key,len);
                }else{
                    lvp_str_dump_with_size(option_start,&val,len);
                    lvp_map_add(options,key,val,NULL,NULL,1);
                    lvp_mem_free(key);
                    key = NULL;
                }
                is_key = 0;
                option_start = str+1;
            }
        }
        str++;
    }
    option_end = str;
    if(option_start<option_end && key != NULL){
        int len = option_end - option_start;
        lvp_str_dump_with_size(option_start,&val,len);
        lvp_map_add(options,key,val,NULL,NULL,1);
    }
    if(key != NULL){
        lvp_mem_free(key);
    }
    
    return LVP_OK;
    
}