#include "lvp_mutex.h"


#ifdef LVP_WIN

LVP_BOOL lvp_mutex_create(lvp_mutex* mutex)
{
    if (mutex == LVP_NULL)
    {
        return LVP_FALSE;
    }
    InitializeCriticalSection(mutex);
    return LVP_TRUE;
}

LVP_BOOL lvp_mutex_free(lvp_mutex* mutex)
{
    if (mutex == LVP_NULL)
    {
        return LVP_FALSE;
    }
    DeleteCriticalSection(mutex);
    return LVP_TRUE;
}

LVP_BOOL lvp_mutex_lock(lvp_mutex* mutex)
{
    if (mutex == LVP_NULL)
    {
        return LVP_FALSE;
    }
    EnterCriticalSection(mutex);

    return LVP_TRUE;
}

LVP_BOOL lvp_mutex_try_lock(lvp_mutex* mutex)
{
    if (mutex == LVP_NULL)
    {
        return LVP_FALSE;
    }
    return TryEnterCriticalSection(mutex);
}

LVP_BOOL lvp_mutex_unlock(lvp_mutex* mutex)
{
    if (mutex == LVP_NULL)
    {
        return LVP_FALSE;
    }
    LeaveCriticalSection(mutex);
    return LVP_TRUE;
}


#endif // WINDOWS_SYSTEM

#ifdef LVP_LINUX

#include <semaphore.h>

LVP_BOOL lvp_mutex_create(lvp_mutex* mutex)
{
    return 0 == pthread_mutex_init(mutex, NULL) ? LVP_TRUE : LVP_FALSE;
}

LVP_BOOL lvp_mutex_free(lvp_mutex* mutex)
{
    return 0 == pthread_mutex_destroy(mutex) ? LVP_TRUE : LVP_FALSE;
}

LVP_BOOL lvp_mutex_lock(lvp_mutex* mutex)
{
    return 0 == pthread_mutex_lock(mutex) ? LVP_TRUE : LVP_FALSE;
}

LVP_BOOL lvp_mutex_try_lock(lvp_mutex* mutex)
{
    return 0 == pthread_mutex_trylock(mutex) ? LVP_TRUE : LVP_FALSE;
}

LVP_BOOL lvp_mutex_unlock(lvp_mutex* mutex)
{
    return 0 == pthread_mutex_unlock(mutex) ? LVP_TRUE : LVP_FALSE;
}
#endif // __linux__
