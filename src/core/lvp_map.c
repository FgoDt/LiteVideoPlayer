#include "lvp_map.h"

static inline uint64_t lvp_map_hash(const char *key, int size){
    uint64_t hash = 0;
    if(key)
    {
        while(*key)
        {
            hash += *key++;
            hash += hash << 10;
            hash ^= hash >> 8;
        }
    }
    return hash % size;
}

LVPMap *lvp_map_alloc(){
    LVPMap *map = lvp_mem_mallocz(sizeof(*map));
    return map;
}

static LVPMapEntry *lvp_map_entry_alloc(){
    LVPMapEntry *entry = lvp_mem_mallocz(sizeof(*entry));
    return entry;
}

static void lvp_map_entry_free(LVPMapEntry *entry){
    assert(entry);
    if(entry->need_free){
        if(entry->cfree){
            entry->cfree(entry->val,entry->usr_data);
        }else{
            lvp_mem_free(entry->val);
        }
    }
    if(entry->key){
        lvp_mem_free(entry->key);
    }
    lvp_mem_free(entry);
}

static void lvp_map_clear(LVPMap *map){
    //assert(map->entrys);
    for (size_t i = 0; i < map->size; i++)
    {
        LVPMapEntry *root = map->entrys[i];
        LVPMapEntry *entry = root;
        while (entry)
        {
            root = entry->next;
            lvp_map_entry_free(entry);
            entry = root;
        }
    }
    lvp_mem_free(map->entrys);
}

void lvp_map_free(LVPMap *map){
    lvp_map_clear(map);
    lvp_mem_free(map);
}

static void init_map(LVPMap *m, int size){
    m->entrys = NULL;
    if(size>0){
        m->entrys = (LVPMapEntry**)lvp_mem_calloc(size,sizeof(*m->entrys));
        if(!m->entrys){
            size = 0;
        }
    }
    m->size = size;
}

int lvp_map_add(LVPMap *map, const char *key,
                void *val, void *usr_data,
                 lvp_custom_free free,LVP_BOOL need_free ){
    assert(map);
    assert(key);
    assert(val);

    if(!map->entrys)
    {
        init_map(map,1);
    }

    int loc = lvp_map_hash(key,map->size);

    //check same key 
    LVPMapEntry *root = map->entrys[loc];
    while (root)
    {
        if(!strcmp(root->key,key)){
            if(root->need_free){
                if(root->cfree){
                    root->cfree(root->val,root->usr_data);
                }
                else
                {
                    lvp_mem_free(root->val);
                }
            }
            root->val = val;
            root->need_free = need_free;
            root->cfree = free;
            return LVP_OK;
        }
        root = root->next;
    }

    
    LVPMapEntry *entry = lvp_map_entry_alloc();
    //fixme when no mem
    lvp_str_dump(key,&entry->key);

    entry->val = val;
    entry->need_free = need_free;
    entry->usr_data = usr_data;
    entry->cfree = free;
    entry->next = map->entrys[loc];
    map->entrys[loc] = entry;

    int num = 0;
    for (; entry != NULL;num++) 
        entry = entry->next;
    
    if(num>2){
        LVPMap  new_map;
        int new_size = ((map->size+2)*3)/2;
        init_map(&new_map,new_size);

        int i ;
        for ( i = 0; i < map->size; i++)
        {
            entry = map->entrys[i];
            while (entry)
            {
                lvp_map_add(&new_map,entry->key,entry->val,
                entry->usr_data,entry->cfree,entry->need_free);
                //just free lvpmapentry struct 
                entry->need_free = LVP_FALSE;
                entry = entry->next;
            }
        }

        lvp_map_clear(map);
        map->size = new_map.size;
        map->entrys = new_map.entrys;
    }

    return LVP_OK;
}

int lvp_map_del(LVPMap *map, const char *key){
    assert(map);
    assert(key);
    if(!map->entrys)
    {
        return LVP_E_NO_MEDIA;
    }

    int loc = lvp_map_hash(key,map->size);
    LVPMapEntry *entry = map->entrys[loc];
    LVPMapEntry *pre = NULL;

    while (entry)
    {
        if(!strcmp(entry->key,key)){
            if(pre == NULL){
                map->entrys[loc] = entry->next;
            }else{
                pre->next = entry->next;
            }
            lvp_map_entry_free(entry);
            return LVP_OK;
        }
        pre = entry;
        entry = entry->next;
    }
    return LVP_E_NO_MEDIA;
    
}

void* lvp_map_get(LVPMap *map, const char *key){
    assert(map);
    assert(key);
    if(!map->entrys){
        return NULL;
    }

    int loc = lvp_map_hash(key,map->size);
    LVPMapEntry *entry = map->entrys[loc];

    while (entry)
    {
        if(!strcmp(entry->key,key)){
            return entry->val;
        }
        entry = entry->next;
    }
    return NULL;
    
}
