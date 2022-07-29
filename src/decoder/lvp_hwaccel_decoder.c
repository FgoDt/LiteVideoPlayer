#include "lvp_hwaccel_decoder.h"
#include <libavcodec/avcodec.h>

static int hw_init(LVPDecoder *decoder)
{
	int err = 0;
	if ((err = av_hwdevice_ctx_create(&decoder->hw_device_ctx, decoder->hw_dev_type, NULL, NULL, 0)) < 0) {
		lvp_waring(decoder->log, "failed to create specified hw device", NULL);
		return err;
	}
	decoder->avctx->hw_device_ctx = av_buffer_ref(decoder->hw_device_ctx);
	return LVP_OK;
}

static enum AVPixelFormat get_hw_format(AVCodecContext* ctx, const enum AVPixelFormat* pix_fmts) {
	const enum AVPixelFormat* p;

	
	for ( p = pix_fmts; *p != AV_PIX_FMT_NONE; p++)
	{
		if (*p == ctx->pix_fmt)
			return *p;
	}
//	lvp_error(decoder->log, "get hw format error", NULL);
	return AV_PIX_FMT_NONE;
}

int lvp_set_up_hwaccel_decoder(LVPDecoder* decoder)
{
	decoder->hw_dev_type = AV_HWDEVICE_TYPE_NONE;
	const char* hwname = lvp_map_get(decoder->options, "lvphw");
	if (hwname) {
		decoder->hw_dev_type = av_hwdevice_find_type_by_name(hwname);
	}
	for (size_t i = 0; ; i++)
	{
		const AVCodecHWConfig* config = avcodec_get_hw_config(decoder->codec, i);
		if (!config) {
			lvp_waring(decoder->log, "NO hw eglConfig find", NULL);
			return LVP_E_NO_MEDIA;
		}
		if (config->methods & AV_CODEC_HW_CONFIG_METHOD_HW_DEVICE_CTX) {
			if (decoder->hw_dev_type == AV_HWDEVICE_TYPE_NONE) {
				decoder->hw_pix_fmt = config->pix_fmt;
				decoder->hw_dev_type = config->device_type;
				break;
			}
			else {
				if (decoder->hw_dev_type == config->device_type) {
					decoder->hw_pix_fmt = config->pix_fmt;
					break;
				}
			}
			hwname = av_hwdevice_get_type_name(decoder->hw_dev_type);
		}
	}
	decoder->avctx->get_format = get_hw_format;
	decoder->avctx->pix_fmt = decoder->hw_pix_fmt;
	int ret = hw_init(decoder);
	if (ret != LVP_OK) {
		return LVP_E_FATAL_ERROR;
	}
	lvp_debug(NULL,"use hw:%s",hwname);
	return LVP_OK;;
}
