/*
 * Copyright (C) 2009 Andrzej K. Haczewski <ahaczewski@gmail.com>
 *
 * DISCLAIMER: The implementation is Git-specific, it is subset of original
 * Pthreads API, without lots of other features that Git doesn't use.
 * Git also makes sure that the passed arguments are valid, so there's
 * no need for double-checking.
 */

#include "../../git-compat-util.h"
#include "pthread.h"

#include <errno.h>
#include <limits.h>

static unsigned __stdcall win32_start_routine(void *arg)
{
	pthread_t *thread = arg;
	thread->tid = GetCurrentThreadId();
	thread->arg = thread->start_routine(thread->arg);
	return 0;
}

int pthread_create(pthread_t *thread, const void *unused,
		   void *(*start_routine)(void*), void *arg)
{
	thread->arg = arg;
	thread->start_routine = start_routine;
	thread->handle = (HANDLE)
		_beginthreadex(NULL, 0, win32_start_routine, thread, 0, NULL);

	if (!thread->handle)
		return errno;
	else
		return 0;
}

int win32_pthread_join(pthread_t *thread, void **value_ptr)
{
	DWORD result = WaitForSingleObject(thread->handle, INFINITE);
	switch (result) {
		case WAIT_OBJECT_0:
			if (value_ptr)
				*value_ptr = thread->arg;
			return 0;
		case WAIT_ABANDONED:
			return EINVAL;
		default:
			return err_win_to_posix(GetLastError());
	}
}

pthread_t pthread_self(void)
{
	pthread_t t = { NULL };
	t.tid = GetCurrentThreadId();
	return t;
}

// Condition variable implementation from
// https://github.com/meganz/mingw-std-threads

void pthread_cond_init_(CONDITION_VARIABLE* cvar)
{
    InitializeCriticalSection(&cvar->mutex);

    cvar->semaphore = CreateSemaphoreA(NULL, 0, 0xFFFF, NULL);
    if (cvar->semaphore == NULL) {
        fprintf(stderr, "FATAL: can't create semaphore (0x%08X).\n", GetLastError());
        abort();
    }

    cvar->wakeEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    if (cvar->wakeEvent == NULL) {
        fprintf(stderr, "FATAL: can't create event (0x%08X).\n", GetLastError());
        CloseHandle(cvar->semaphore);
        abort();
    }
}

void pthread_cond_destroy(CONDITION_VARIABLE* cvar)
{
    DeleteCriticalSection(&cvar->mutex);
    CloseHandle(cvar->wakeEvent);
    CloseHandle(cvar->semaphore);
}

BOOL pthread_cond_wait_(CONDITION_VARIABLE* cvar, CRITICAL_SECTION* cs, DWORD dwMilliseconds)
{
    EnterCriticalSection(&cvar->mutex);
    InterlockedIncrement(&cvar->numWaiters);
    LeaveCriticalSection(&cvar->mutex);

    LeaveCriticalSection(cs);
    DWORD ret = WaitForSingleObject(cvar->semaphore, dwMilliseconds);
    InterlockedDecrement(&cvar->numWaiters);
    SetEvent(cvar->wakeEvent);
    EnterCriticalSection(cs);

    if (ret == WAIT_OBJECT_0)
        return TRUE;
    else if (ret == WAIT_TIMEOUT)
        return FALSE;
    else {
        //2 possible cases:
        //1)The point in notify_all() where we determine the count to
        //increment the semaphore with has not been reached yet:
        //we just need to decrement mNumWaiters, but setting the event does not hurt
        //
        //2)Semaphore has just been released with mNumWaiters just before
        //we decremented it. This means that the semaphore count
        //after all waiters finish won't be 0 - because not all waiters
        //woke up by acquiring the semaphore - we woke up by a timeout.
        //The notify_all() must handle this gracefully
        //
        fprintf(stderr, "FATAL: can't wait on condition variable.\n");
        abort();
    }
}

void pthread_cond_signal(CONDITION_VARIABLE* cvar)
{
    EnterCriticalSection(&cvar->mutex);
    LONG targetWaiters = InterlockedExchangeAdd(&cvar->numWaiters, 0) - 1;
    if (targetWaiters <= -1) {
        LeaveCriticalSection(&cvar->mutex);
        return;
    }
    ReleaseSemaphore(cvar->semaphore, 1, NULL);
    while(cvar->numWaiters > targetWaiters)
    {
        auto ret = WaitForSingleObject(cvar->wakeEvent, 1000);
        if (ret == WAIT_FAILED || ret == WAIT_ABANDONED) {
            fprintf(stderr, "FATAL: can't wait on wake event.\n");
            abort();
        }
    }
    assert(cvar->numWaiters == targetWaiters);
    LeaveCriticalSection(&cvar->mutex);
}

void pthread_cond_broadcast(CONDITION_VARIABLE* cvar)
{
    // block any further wait requests until all current waiters are unblocked
    EnterCriticalSection(&cvar->mutex);
    if (InterlockedExchangeAdd(&cvar->numWaiters, 0) <= 0) {
        LeaveCriticalSection(&cvar->mutex);
        return;
    }

    ReleaseSemaphore(cvar->semaphore, cvar->numWaiters, NULL);
    while(cvar->numWaiters > 0)
    {
        auto ret = WaitForSingleObject(cvar->wakeEvent, 1000);
        if (ret == WAIT_FAILED || ret == WAIT_ABANDONED) {
            fprintf(stderr, "FATAL: can't wait on wake event.\n");
            abort();
        }
    }
    assert(cvar->numWaiters == 0);
    //in case some of the waiters timed out just after we released the
    //semaphore by mNumWaiters, it won't be zero now, because not all waiters
    //woke up by acquiring the semaphore. So we must zero the semaphore before
    //we accept waiters for the next event
    while(WaitForSingleObject(cvar->semaphore, 0) == WAIT_OBJECT_0);
    LeaveCriticalSection(&cvar->mutex);
}
