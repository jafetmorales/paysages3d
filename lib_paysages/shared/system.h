#ifndef _PAYSAGES_SYSTEM_H_
#define _PAYSAGES_SYSTEM_H_

#include <glib.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef GThread Thread;
typedef void*(*ThreadFunction)(void* data);

static inline void systemInit()
{
    g_thread_init(NULL);
}

static inline Thread* threadCreate(ThreadFunction function, void* data)
{
    GError* error;
    return g_thread_create((GThreadFunc)function, data, 1, &error);
}

static inline void* threadJoin(Thread* thread)
{
    return g_thread_join(thread);
}

typedef GMutex Mutex;

static inline Mutex* mutexCreate()
{
    return g_mutex_new();
}

static inline void mutexDestroy(Mutex* mutex)
{
    g_mutex_free(mutex);
}

static inline void mutexAcquire(Mutex* mutex)
{
    g_mutex_lock(mutex);
}

static inline void mutexRelease(Mutex* mutex)
{
    g_mutex_unlock(mutex);
}

static inline void timeSleepMs(unsigned long ms)
{
    g_usleep(ms * 1000);
}

#ifdef __cplusplus
}
#endif

#endif
