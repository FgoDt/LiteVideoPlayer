#include "lvp_semaphore.h"

#ifdef LVP_WIN32

#include <Windows.h>

lvp_semaphore lvp_semaphore_create(int initCount, int maxCount)
{
    lvp_semaphore semaphore = CreateSemaphore(NULL, initCount, maxCount, NULL);
    return semaphore;
}

LVP_BOOL lvp_semaphore_free(lvp_semaphore semaphore)
{
    if (semaphore == LVP_NULL)
    {
        return LVP_FALSE;
    }

    return CloseHandle(semaphore);
}

LVP_BOOL lvp_semaphore_wait_one(lvp_semaphore semaphore)
{
    if (semaphore == LVP_NULL)
    {
        return LVP_FALSE;
    }
    return WAIT_OBJECT_0 == WaitForSingleObject(semaphore, INFINITE) ? LVP_TRUE : LVP_FALSE;
}

LVP_BOOL lvp_semaphore_try_wait_one(lvp_semaphore semaphore)
{
    if (semaphore == LVP_NULL)
    {
        return LVP_FALSE;
    }

    return WAIT_OBJECT_0 == WaitForSingleObject(semaphore, 0) ? LVP_TRUE : LVP_FALSE;
}

LVP_BOOL lvp_semaphore_wait(lvp_semaphore semaphore, int count)
{
    if (semaphore == LVP_NULL)
    {
        return LVP_FALSE;
    }

    int waited = 0;
    for (int i = 0; i < count; i++)
    {
        if (lvp_semaphore_wait_one(semaphore))
        {
            waited++;
        }
        else
        {
            break;
        }
    }

    if (waited==count)
    {
        return LVP_TRUE;
    }

    lvp_semaphore_release(semaphore, waited);

    return LVP_FALSE;
}

LVP_BOOL lvp_semaphore_try_wait(lvp_semaphore semaphore, int count)
{
    if (semaphore == LVP_NULL)
    {
        return LVP_FALSE;
    }

    int waited = 0;

    for (int i = 0; i < count; i++)
    {
        if (lvp_semaphore_try_wait_one(semaphore))
        {
            waited++;
        }
        else
        {
            break;
        }
    }

    if (waited==count)
    {
        return LVP_TRUE;
    }

    lvp_semaphore_release(semaphore, waited);

    return LVP_FALSE;
}

LVP_BOOL lvp_semaphore_release_one(lvp_semaphore semaphore)
{
    if (semaphore == LVP_NULL)
    {
        return LVP_FALSE;
    }

    return ReleaseSemaphore(semaphore, 1, LVP_NULL);
}

LVP_BOOL lvp_semaphore_release(lvp_semaphore semaphore, int count)
{
    if (semaphore == LVP_NULL)
    {
        return LVP_FALSE;
    }

    return ReleaseSemaphore(semaphore, count, LVP_NULL);
}


#endif // LVP_WIN32

#ifdef LVP_LINUX

#include <semaphore.h>

lvp_semaphore lvp_semaphore_create(int initCount, int maxCount)
{
    lvp_semaphore semaphore;
    sem_init(&semaphore,0,initCount);
    return semaphore;

}
LVP_BOOL lvp_semaphore_free(lvp_semaphore semaphore)
{
    if (0 == sem_destroy(&semaphore))
    {
        return LVP_TRUE;
    }

    return LVP_FALSE;
}
LVP_BOOL lvp_semaphore_wait_one(lvp_semaphore semaphore)
{
    return 0 == sem_wait(&semaphore) ? LVP_TRUE : LVP_FALSE;
}

LVP_BOOL lvp_semaphore_try_wait_one(lvp_semaphore semaphore) 
{
    return 0 == sem_trywait(&semaphore) ? LVP_TRUE : LVP_FALSE;
}

LVP_BOOL lvp_semaphore_wait(lvp_semaphore semaphore, int count)
{
    int waited = 0;

    for (int i = 0; i < count; i++)
    {
        if (lvp_semaphore_wait_one(semaphore))
        {
            waited++;
        }
        else
        {
            break;
        }
    }

    if (waited == count)
    {
        return LVP_TRUE;
    }

    lvp_semaphore_release(semaphore, waited);

    return LVP_FALSE;
}

LVP_BOOL lvp_semaphore_try_wait(lvp_semaphore semaphore, int count)
{
    int waited = 0;

    for (int i = 0; i < count; i++)
    {
        if (lvp_semaphore_try_wait_one(semaphore))
        {
            waited++;
        }
        else
        {
            break;
        }
    }

    if (waited == count)
    {
        return LVP_TRUE;
    }

    lvp_semaphore_release(semaphore, waited);

    return LVP_FALSE;
}

LVP_BOOL lvp_semaphore_release_one(lvp_semaphore semaphore) 
{
    return 0 == sem_post(&semaphore) ? LVP_TRUE : LVP_FALSE;
}

LVP_BOOL lvp_semaphore_release(lvp_semaphore semaphore, int count)
{
    for (int i = 0; i < count; i++)
    {
        if (!lvp_semaphore_release_one(semaphore))
        {
            return LVP_FALSE;
        }
    }
    return LVP_TRUE;
}

#endif // LVP_LINUX

