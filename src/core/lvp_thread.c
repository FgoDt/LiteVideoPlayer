#include "lvp_thread.h"

#ifdef LVP_WIN

LVP_BOOL lvp_thread_create(lvp_thread* thread, lvp_thread_func func, void * param)
{
    if (thread==LVP_NULL)
    {
        return LVP_FALSE;
    }
    HANDLE handle = CreateThread(NULL, 0, (PTHREAD_START_ROUTINE)func, param, 0, NULL);
    if (handle != LVP_NULL)
    {
        *thread = handle;
        return LVP_TRUE;
    }

    return LVP_FALSE;
}

LVP_BOOL lvp_thread_join(lvp_thread lvpThread)
{
    if (LVP_NULL== lvpThread)
    {
        return LVP_FALSE;
    }

    if (WaitForSingleObject(lvpThread, INFINITE) != WAIT_FAILED)
    {
        if (CloseHandle(lvpThread))
        {
            return LVP_TRUE;
        }
    }

    return LVP_FALSE;
}

#endif

#ifdef LVP_LINUX

LVP_BOOL lvp_thread_create(lvp_thread* thread, lvp_thread_func func, void *param)
{
    return 0 == pthread_create((pthread_t*)thread, 0, func, param) ? LVP_TRUE : LVP_FALSE;
}
LVP_BOOL lvp_thread_join(lvp_thread lvpThread)
{
    return 0 == pthread_join(lvpThread, LVP_NULL);
}

#endif // LVP_LINUX


