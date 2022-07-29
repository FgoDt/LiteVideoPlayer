#ifndef LVP_THREAD_H_
#define LVP_THREAD_H_

#include "lvp_core.h"

typedef void*(*lvp_thread_func)(void *param);
#ifdef LVP_WIN32
typedef void* lvp_thread;
#endif // LVP_WIN32

#ifdef LVP_LINUX
typedef long int lvp_thread;
#endif // LVP_LINUX


LVP_BOOL lvp_thread_create(lvp_thread* thread,lvp_thread_func func, void *param);
LVP_BOOL lvp_thread_join(lvp_thread lvpThread);

#endif // !LVP_THREAD_H_


