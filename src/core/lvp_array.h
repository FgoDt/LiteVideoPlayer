#ifndef _LVP_ARRAY_H_
#define _LVP_ARRAY_H_

#include "lvp_core.h"
#include "lvp.h"

typedef struct lvp_array_s{
    size_t count;
    void **elems;
}LVPArray;



#endif