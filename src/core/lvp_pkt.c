#include "lvp_pkt.h"
#include "lvp_mem.h"
#include <libavcodec/avcodec.h>

LVPPkt* lvp_pkt_alloc(AVPacket* pkt)
{
	LVPPkt* lpkt = lvp_mem_mallocz(sizeof(*lpkt));
	lpkt->pkt = pkt;
	return lpkt;
}

LVPPkt* lvp_pkt_clone(LVPPkt* lpkt)
{
	LVPPkt* cpkt = lvp_pkt_alloc(av_packet_clone(lpkt->pkt));
	cpkt->width = lpkt->width;
	cpkt->height = lpkt->height;
	cpkt->type = lpkt->type;
	cpkt->extra_data = lpkt->extra_data;
	cpkt->time_base = lpkt->time_base;
	cpkt->codec_id = lpkt->codec_id;
	return cpkt;
}

void lvp_pkt_free(LVPPkt* lvp_pkt)
{
	if (!lvp_pkt) {
		return;
	}
	lvp_mem_free(lvp_pkt);
}
