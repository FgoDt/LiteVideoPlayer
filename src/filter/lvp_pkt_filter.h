#ifndef _LVP_PKT_FILTER_H_
#define _LVP_PKT_FILTER_H_
#include "../core/lvp_core.h"
#include <libavcodec/avcodec.h>
#include "../core/lvp_pkt.h"

typedef struct lvp_pkt_filter{
    LVPLog *log;
    LVPEventControl *ctl;

    //when pkt cache full we need handle this packet 
    //send later
	LVPPkt* filtered_pkt;

    LVPList *modules;
}LVPPktFilter;

#endif