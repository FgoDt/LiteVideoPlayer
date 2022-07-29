#include "lvp_hwaccel.h"
#include "../core/lvp_map.h"
#include <libavutil/hwcontext.h>
#include <libavutil/avutil.h>
#include <libavcodec/avcodec.h>
#include <libavcodec/utils.h>

LVPHWAccel *lvp_hwaccel_get_hwaccel(LVPMap *options){
    LVPHWAccel *hw = (LVPHWAccel*)lvp_mem_mallocz(sizeof(*hw));
    const char *hwaccel = lvp_map_get(options,"hwaccel");
	hw->id = LVP_HWACCEL_NONE;
    if(!strcmp(hwaccel,"auto")){
        hw->id = LVP_HWACCEL_AUTO;
    }else{
        enum AVHWDeviceType type;
        int i;
        for (size_t i = 0; i < hwaccels[i].name; i++)
        {
            if(!strcmp(hwaccels[i].name,hwaccel)){
                hw->id = hwaccels[i].id;
                break;
            }
        }

        if(!hw->id){
            type = av_hwdevice_find_type_by_name(hwaccel);
            if(!type != AV_HWDEVICE_TYPE_NONE){
                hw->id = LVP_HWACCEL_GENERIC;
                hw->type = type;
                lvp_str_dump(hwaccel,&hw->name);
            }
        }

        if(!hw->id){
            lvp_waring(NULL,"lvp get hwaccel error for %s",hwaccel);
            lvp_mem_free(hw);
            return NULL;
        }

        hw->pix_fmt = AV_PIX_FMT_NONE;
        
    }

    return hw;
    
}

int lvp_hwaccel_set_up(AVCodecContext* avctx, LVPHWAccel* hwaccel) {
	assert(avctx);
	assert(hwaccel);

    int err;
    int auto_device = 0;
    enum AVHWDeviceType type;
    AVBufferRef *device_ref = NULL;

	switch (hwaccel->id)
	{
	case LVP_HWACCEL_AUTO:
        auto_device = 1;
		break;
    case LVP_HWACCEL_GENERIC:
        type = hwaccel->type;
        err = av_hwdevice_ctx_create(&device_ref,type,hwaccel->name,NULL,0);
        break;
    case LVP_HWACCEL_VIDEOTOOLBOX:
        return LVP_OK;
        break;
	case LVP_HWACCEL_QSV:
        return LVP_OK;
		break;
    case LVP_HWACCEL_CUVID:
        return LVP_OK;
        break;
	default:
        return LVP_OK;
		break;
	}

    if(auto_device){
        int i;
        if(!avcodec_get_hw_config(avctx,0)){
            return LVP_OK;
        }
        while (1)
        {
            AVCodecHWConfig *config = avcodec_get_hw_config()
            
        }
        
    }

}