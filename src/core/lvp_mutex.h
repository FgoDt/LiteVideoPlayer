#ifndef _LVP_MUTEX_H_
#define _LVP_MUTEX_H_
#include "lvp_core.h"




LVP_BOOL lvp_mutex_create(lvp_mutex* mutex);
LVP_BOOL lvp_mutex_free(lvp_mutex* mutex);
LVP_BOOL lvp_mutex_lock(lvp_mutex* mutex);
LVP_BOOL lvp_mutex_try_lock(lvp_mutex* mutex);
LVP_BOOL lvp_mutex_unlock(lvp_mutex* mutex);

#endif // !LVP_MUTEX_H_




