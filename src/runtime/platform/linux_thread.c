/*
 * the MIT License (MIT)
 *
 * Copyright (c) 2023 Trent Summerlin
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * the above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
#if defined(__linux__)

#include "thread.h"

#include <stdio.h>
#include <string.h>

#define _MULTI_THREADED
#include <pthread.h>

#include "../error.h"

typedef struct PLAT_RBTK_THREAD {
    pthread_t handle;
} PLAT_RBTK_THREAD;

static __thread RBTK_THREAD *thread_tls;
static PLAT_RBTK_THREAD plat_main_thread;
static bool initialized;

static void *
start_thread(void *params) {
    RBTK_THREAD *thread = params;

    thread_tls = thread;

    thread->running = true;
    thread->entrypoint(thread->params);
    thread->running = false;

    return NULL;
}

RBTK_PLATFORM RBTK_NO_DISCARD bool
plat_rbtk_thread_init(void)
{
    if (initialized) {
        return true;
    }

    plat_main_thread.handle = pthread_self();
    RBTK_MAIN_THREAD->plat = &plat_main_thread;
    thread_tls = RBTK_MAIN_THREAD;

    initialized = true;
    return true;
}

RBTK_PLATFORM RBTK_NO_DISCARD bool
plat_rbtk_thread_terminate(void)
{
    if (!initialized) {
        return true;
    }

    /* POSIX threads terminates by itself */

    initialized = false;
    return true;
}

RBTK_NO_DISCARD RBTK_THREAD *
plat_rbtk_current_thread(void)
{
    RBTK_THREAD *info = thread_tls;
    if (!info) {
        rbtk_signal_error(RBTK_ERROR_UNEXPECTED_STATE,
            "could not determine current thread");
        return NULL;
    }
    return info;
}

RBTK_NO_DISCARD bool
plat_rbtk_create_thread(RBTK_THREAD *thread)
{
    assert(thread);

    PLAT_RBTK_THREAD *plat = NULL;
    RBTK_MALLOC_OR_RETURN(&plat, false,
        "failed to allocate memory for POSIX thread");

    plat->handle = 0x00;
    thread->plat = plat;

    return true;
}

RBTK_NO_DISCARD bool
plat_rbtk_destroy_thread(RBTK_THREAD *thread)
{
    PLAT_RBTK_THREAD *plat = thread->plat;
    if (thread->running) {
        pthread_cancel(plat->handle);
        thread->running = false;
    }
    return true;
}

RBTK_PLATFORM bool
plat_rbtk_apply_thread_priority(RBTK_THREAD *thread)
{
    assert(thread);

    PLAT_RBTK_THREAD *plat = thread->plat;

    int priority = 0;
    switch (thread->priority) {
    case RBTK_THREAD_PRIORITY_BACKGROUND:
        priority = 0;
        break;
    case RBTK_THREAD_PRIORITY_LOW:
        priority = 1;
        break;
    case RBTK_THREAD_PRIORITY_BELOW_NORMAL:
        priority = 2;
        break;
    case RBTK_THREAD_PRIORITY_NORMAL:
        priority = 3;
        break;
    case RBTK_THREAD_PRIORITY_ABOVE_NORMAL:
        priority = 4;
        break;
    case RBTK_THREAD_PRIORITY_HIGH:
        priority = 5;
        break;
    case RBTK_THREAD_PRIORITY_CRITICAL:
        priority = 6;
        break;
    }

    return !pthread_setschedprio(plat->handle, priority);
}

RBTK_NO_DISCARD bool
plat_rbtk_start_thread(RBTK_THREAD *thread)
{
    assert(thread);

    pthread_t thread_handle = 0x00;
    if (pthread_create(&thread_handle, NULL,
    		start_thread, thread)) {
        rbtk_signal_error(RBTK_ERROR_PLATFORM,
            "failed to create POSIX thread");
        return false;
    }

    PLAT_RBTK_THREAD *plat = thread->plat;
    plat->handle = thread_handle;
    return true;
}

RBTK_NO_DISCARD bool
plat_rbtk_stop_thread(RBTK_THREAD *thread)
{
    assert(thread);
    PLAT_RBTK_THREAD *plat = thread->plat;
    if (pthread_cancel(plat->handle))
    {
        rbtk_signal_error(RBTK_ERROR_PLATFORM,
            "failed to terminate POSIX thread");
        return false;
    }
    thread->running = false;
    return true;
}

void
plat_rbtk_yield_thread(RBTK_THREAD *thread)
{
    assert(thread);
    rbtk_signal_error(RBTK_ERROR_UNSUPPORTED,
            "thread yielding not yet implemented for Linux");
}

RBTK_NO_DISCARD bool
plat_rbtk_create_thread_storage(RBTK_THREAD_STORAGE_KEY *key)
{
    assert(key);
    rbtk_signal_error(RBTK_ERROR_UNSUPPORTED,
            "thread local storage not yet implemented for Linux");
    return false;
}

RBTK_NO_DISCARD bool
plat_rbtk_destroy_thread_storage(RBTK_THREAD_STORAGE_KEY *key)
{
    assert(key);
    rbtk_signal_error(RBTK_ERROR_UNSUPPORTED,
            "thread local storage not yet implemented for Linux");
    return false;
}

RBTK_NO_DISCARD void *
plat_rbtk_get_thread_storage(RBTK_THREAD_STORAGE_KEY *key)
{
    assert(key);
    rbtk_signal_error(RBTK_ERROR_UNSUPPORTED,
            "thread local storage not yet implemented for Linux");
    return NULL;
}

#endif

