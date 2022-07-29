#ifndef _LVP_CACHE_MODULE_H_
#define _LVP_CACHE_MODULE_H_

#include "../core/lvp_core.h"
#include <libavformat/avformat.h>

#define CACHE_TYPE_PKT      0
#define CACHE_TYPE_FRAME    1
#define CACHE_TYPE_SUBTITLE 2

#define PKT_SIZE 300
#define FRAME_SIZE 10
typedef struct lvp_cache_module
{
    LVPLog *log;
    LVPEventControl *ctl;

	lvp_mutex mutex;
    LVPNQueue *data;
    int type;
    int media_type;
    int max_size;

	long response_data;
}LVPCache;


#endif