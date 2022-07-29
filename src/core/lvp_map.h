#ifndef _LVP_MAP_H_
#define _LVP_MAP_H_

#include "lvp_core.h"
#include "lvp.h"

typedef struct lvp_map_entry{
    char                    *key; ///< key
    void                    *val; ///< data
    void                    *usr_data; ///< entry usr data
    lvp_custom_free         cfree; ///< data custom free func
    LVP_BOOL                need_free; ///< data need free
    struct lvp_map_entry    *next; 
}LVPMapEntry;

struct lvp_map{
    int             size; ///< map max entrys, size will change when entrys.num >= size
    LVPMapEntry     **entrys; ///< map entrys
};


LVPMap *lvp_map_alloc();

void lvp_map_free(LVPMap *map);

int lvp_map_add(LVPMap *map, const char *key,
                void *val, void *usr_data,
                 lvp_custom_free free,LVP_BOOL need_free );

int lvp_map_del(LVPMap *map, const char *key);

void* lvp_map_get(LVPMap *map, const char *key);

#endif