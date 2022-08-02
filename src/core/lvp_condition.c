#include "lvp_condition.h"

#ifdef LVP_WIN

#include <Windows.h>

LVP_BOOL lvp_condition_create(lvp_condition* cond)
{
    if (LVP_NULL==cond)
    {
        return LVP_FALSE;
    }
    InitializeConditionVariable(cond);
    return LVP_TRUE;
}

LVP_BOOL lvp_condition_free(lvp_condition* cond)
{
    return LVP_TRUE;
}

LVP_BOOL lvp_condition_notify(lvp_condition* cond)
{
    WakeConditionVariable(cond);
    return LVP_TRUE;
}

LVP_BOOL lvp_condition_notify_all(lvp_condition* cond)
{
    WakeAllConditionVariable(cond);
    return LVP_TRUE;
}

LVP_BOOL lvp_condition_wait(lvp_condition* cond, lvp_mutex* mutex)
{
    return SleepConditionVariableCS(cond, mutex, INFINITE);
}

#endif // LVP_WIN

#ifdef LVP_LINUX

LVP_BOOL      lvp_condition_create(lvp_condition* cond)
{
    if (LVP_NULL==cond)
    {
        return LVP_FALSE;
    }
    return 0 == pthread_cond_init(cond, LVP_NULL) ? LVP_TRUE : LVP_FALSE;
}
LVP_BOOL      lvp_condition_free(lvp_condition* cond)
{
    if (LVP_NULL == cond)
    {
        return LVP_FALSE;
    }
    return 0 == pthread_cond_destroy(cond);
}
LVP_BOOL      lvp_condition_notify(lvp_condition* cond) 
{
    if (LVP_NULL == cond)
    {
        return LVP_FALSE;
    }
    return 0 == pthread_cond_signal(cond) ? LVP_TRUE : LVP_FALSE;
}
LVP_BOOL      lvp_condition_notify_all(lvp_condition* cond)
{
    if (LVP_NULL == cond)
    {
        return LVP_FALSE;
    }

    return 0 == pthread_cond_broadcast(cond) ? LVP_TRUE : LVP_FALSE;
}
LVP_BOOL      lvp_condition_wait(lvp_condition* cond, lvp_mutex* mutex)
{
    if (LVP_NULL == cond)
    {
        return LVP_FALSE;
    }

    return 0 == pthread_cond_wait(cond, mutex) ? LVP_TRUE : LVP_FALSE;
}

#endif // LVP_LINUX

