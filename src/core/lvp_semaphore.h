#ifndef LVP_SEMAPHORE_H_
#define LVP_SEMAPHORE_H_

#include "lvp_core.h"

#ifdef LVP_WIN
typedef void* lvp_semaphore;
#endif // LVP_WIN

#ifdef LVP_LINUX
typedef long int lvp_semaphore;
#endif // LVP_LINUX

lvp_semaphore lvp_semaphore_create(int initCount,int maxCount);
LVP_BOOL lvp_semaphore_free(lvp_semaphore semaphore);
LVP_BOOL lvp_semaphore_wait_one(lvp_semaphore semaphore);
LVP_BOOL lvp_semaphore_try_wait_one(lvp_semaphore semaphore);
LVP_BOOL lvp_semaphore_wait(lvp_semaphore semaphore, int count);
LVP_BOOL lvp_semaphore_try_wait(lvp_semaphore semaphore, int count);
LVP_BOOL lvp_semaphore_release_one(lvp_semaphore semaphore);
LVP_BOOL lvp_semaphore_release(lvp_semaphore semaphore, int count);



#endif // !LVP_SEMAPHORE_H_

