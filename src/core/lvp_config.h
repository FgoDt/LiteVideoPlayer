#ifndef _LVP_CONFIG_H_
#define _LVP_CONFIG_H_

#ifdef LVP_LINUX
#include "lvp_linux_config.h"
#define LVP_USE_PTHREAD  1
#endif

#ifdef LVP_WIN
#include "lvp_win32_config.h"
#endif

#endif