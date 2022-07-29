/*
 * this file use many ffmpeg code 
 * 
 * FFmpeg is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * FFmpeg is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with FFmpeg; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#ifndef _LVP_HWACCEL_H_
#define _LVP_HWACCEL_H_

#include "../core/lvp_map.h"
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>

typedef enum lvp_hwaccel_id{
    LVP_HWACCEL_NONE = 0,
    LVP_HWACCEL_AUTO,
    LVP_HWACCEL_GENERIC,
    LVP_HWACCEL_VIDEOTOOLBOX,
    LVP_HWACCEL_QSV,
    LVP_HWACCEL_CUVID,
}LVPHWAccelID;

typedef struct lvp_hwaccel{
    const char *name;
    int (*init)(AVCodecContext *s);
    LVPHWAccelID id;
    enum AVPixelFormat pix_fmt;
    enum AVHWDeviceType type;
}LVPHWAccel;

typedef struct lvp_hwdevice{
    const char *name;
    enum AVHWDeviceType type;
    AVBufferRef *device_ref;
}LVPHWDevice;

const LVPHWAccel hwaccels[] = {
#if CONFIG_VIDEOTOOLBOX
    { "videotoolbox", videotoolbox_init, HWACCEL_VIDEOTOOLBOX, AV_PIX_FMT_VIDEOTOOLBOX },
#endif
#if CONFIG_LIBMFX
    { "qsv",   qsv_init,   HWACCEL_QSV,   AV_PIX_FMT_QSV },
#endif
#if CONFIG_CUVID
    { "cuvid", cuvid_init, HWACCEL_CUVID, AV_PIX_FMT_CUDA },
#endif
    { 0 }
};


LVPHWAccel *lvp_hwaccel_get_hwaccel(LVPMap *options);

#endif