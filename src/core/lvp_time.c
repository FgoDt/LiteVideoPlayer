
#include "lvp_time.h"

void lvp_sleep(uint64_t ms){
#ifdef LVP_LINUX
    usleep(ms*1000);
#endif

#ifdef LVP_WIN32
    Sleep(ms);
#endif
}