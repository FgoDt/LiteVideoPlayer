#ifndef LVP_CONDITION_H_
#define LVP_CONDITION_H_

#include "lvp_core.h"
#include "lvp_mutex.h"

#ifdef LVP_WIN32
typedef CONDITION_VARIABLE lvp_condition;

#endif

#ifdef LVP_LINUX
typedef pthread_cond_t lvp_condition;
#endif // LVP_LINUX


LVP_BOOL      lvp_condition_create(lvp_condition* cond);
LVP_BOOL      lvp_condition_free(lvp_condition* cond);
LVP_BOOL      lvp_condition_notify(lvp_condition* cond);
LVP_BOOL      lvp_condition_notify_all(lvp_condition* cond);
LVP_BOOL      lvp_condition_wait(lvp_condition* cond,lvp_mutex* mutex);

#endif // !LVP_CONDITION_H_


