#ifndef LVP_SRC_CORE_LVP_PKT_H_
#define LVP_SRC_CORE_LVP_PKT_H_

#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>

typedef struct LVPPkt {
	AVPacket* pkt;
	enum AVMediaType type;
	int width;
	int height;
	int extra_data;
	AVRational time_base;
    enum AVCodecID   codec_id;
}LVPPkt;

LVPPkt* lvp_pkt_alloc(AVPacket *pkt);

LVPPkt* lvp_pkt_clone(LVPPkt* lpkt);

void lvp_pkt_free(LVPPkt* lvp_pkt);

#endif // !LVP_SRC_CORE_LVP_PKT_H_
