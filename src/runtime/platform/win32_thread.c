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
#if defined(_WIN32)

#include "thread.h"

#include <stdio.h>
#include <string.h>

#include <windows.h>

#include "../error.h"

/*
 * This warning is disabled in the project settings but for some reason the
 * compiler completely ignores it.
 */
#pragma warning(disable: 4127)

typedef struct PLAT_RBTK_THREAD {
    DWORD id;
    HANDLE handle;
} PLAT_RBTK_THREAD;

struct storage_block {
    void *data;
    struct storage_block *next;
};

typedef struct PLAT_RBTK_THREAD_STORAGE_KEY {
    DWORD tls_index;
    struct storage_block *blocks;
} PLAT_RBTK_THREAD_STORAGE_KE;

static DWORD thread_tls_index;
static PLAT_RBTK_THREAD plat_main_thread;
static BOOL initialized;

static DWORD WINAPI
start_thread(LPVOID params) {
    RBTK_THREAD *thread = params;

    TlsSetValue(thread_tls_index, thread);

    thread->running = true;
    thread->entrypoint(thread->params);
    thread->running = false;

    return EXIT_SUCCESS;
}

RBTK_PLATFORM RBTK_NO_DISCARD bool
plat_rbtk_thread_init(void)
{
    if (initialized) {
        return true;
    }

    thread_tls_index = TlsAlloc();
    if (thread_tls_index == TLS_OUT_OF_INDEXES) {
        rbtk_signal_error(RBTK_ERROR_PLATFORM,
            "failed to allocate TLS for thread data.");
        return false;
    }

    plat_main_thread.id = GetCurrentThreadId();
    plat_main_thread.handle = GetCurrentThread();
    RBTK_MAIN_THREAD->plat = &plat_main_thread;

    TlsSetValue(thread_tls_index, RBTK_MAIN_THREAD);

    initialized = 1;
    return true;
}

RBTK_PLATFORM RBTK_NO_DISCARD bool
plat_rbtk_thread_terminate(void)
{
    if (!initialized) {
        return true;
    }

    if (!TlsFree(thread_tls_index)) {
        rbtk_signal_error(RBTK_ERROR_PLATFORM,
            "failed to free TLS for thread data.");
        return false;
    }

    initialized = 0;
    return true;
}

RBTK_NO_DISCARD RBTK_THREAD *
plat_rbtk_current_thread(void)
{
    RBTK_THREAD *info = TlsGetValue(thread_tls_index);
    if (!info) {
        DWORD win32_error = GetLastError();
        if(win32_error) {
            SetLastError(win32_error);
            rbtk_signal_error(RBTK_ERROR_PLATFORM,
                "Win32 error %d", win32_error);
        } else {
            rbtk_signal_error(RBTK_ERROR_UNEXPECTED_STATE,
                "could not determine current thread");
        }
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
        "failed to allocate memory for Win32 thread");

    plat->id = 0;
    plat->handle = NULL;
    thread->plat = plat;

    return true;
}

RBTK_NO_DISCARD bool
plat_rbtk_destroy_thread(RBTK_THREAD *thread)
{
    PLAT_RBTK_THREAD *plat = thread->plat;
    if (thread->running) {
        TerminateThread(plat->handle, 0);
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
        priority = THREAD_PRIORITY_LOWEST - 1;
    case RBTK_THREAD_PRIORITY_LOW:
        priority = THREAD_PRIORITY_LOWEST;
        break;
    case RBTK_THREAD_PRIORITY_BELOW_NORMAL:
        priority = THREAD_PRIORITY_BELOW_NORMAL;
        break;
    case RBTK_THREAD_PRIORITY_NORMAL:
        priority = THREAD_PRIORITY_NORMAL;
        break;
    case RBTK_THREAD_PRIORITY_ABOVE_NORMAL:
        priority = THREAD_PRIORITY_ABOVE_NORMAL;
        break;
    case RBTK_THREAD_PRIORITY_HIGH:
        priority = THREAD_PRIORITY_HIGHEST;
        break;
    case RBTK_THREAD_PRIORITY_CRITICAL:
        priority = THREAD_PRIORITY_TIME_CRITICAL;
        break;
    }

    return SetThreadPriority(plat->handle, priority);
}

RBTK_NO_DISCARD bool
plat_rbtk_start_thread(RBTK_THREAD *thread)
{
    assert(thread);

    DWORD thread_id;
    HANDLE thread_handle = CreateThread(NULL, 0, start_thread,
        thread, 0, &thread_id);
    if (!thread_handle) {
        rbtk_signal_error(RBTK_ERROR_PLATFORM,
            "failed to create Win32 thread");
        return false;
    }

    PLAT_RBTK_THREAD *plat = thread->plat;
    plat->id = thread_id;
    plat->handle = thread_handle;
    return true;
}

RBTK_NO_DISCARD bool
plat_rbtk_stop_thread(RBTK_THREAD *thread)
{
    assert(thread);
    PLAT_RBTK_THREAD *plat = thread->plat;
    if (!TerminateThread(plat->handle, 0))
    {
        rbtk_signal_error(RBTK_ERROR_PLATFORM,
            "failed to terminate Win32 thread");
        return false;
    }
    thread->running = false;
    return true;
}

void
plat_rbtk_yield_thread(RBTK_UNUSED RBTK_THREAD *thread)
{
    assert(thread);
    SwitchToThread();
}

RBTK_NO_DISCARD bool
plat_rbtk_create_thread_storage(RBTK_THREAD_STORAGE_KEY *key)
{
    assert(key);

    DWORD tls_index = TlsAlloc();
    if (tls_index == TLS_OUT_OF_INDEXES) {
        rbtk_signal_error(RBTK_ERROR_PLATFORM,
            "failed to allocate TLS for thread storage.");
        return false;
    }

    PLAT_RBTK_THREAD_STORAGE_KEY *plat = NULL;
    RBTK_MALLOC_OR_RETURN(&plat, false,
        "failure to allocate storage key for Win32");

    plat->tls_index = tls_index;
    plat->blocks = NULL;
    key->plat = plat;

    return true;
}

RBTK_NO_DISCARD bool
plat_rbtk_destroy_thread_storage(RBTK_THREAD_STORAGE_KEY *key)
{
    assert(key);

    PLAT_RBTK_THREAD_STORAGE_KEY *plat = key->plat;

    struct storage_block *cur_block = plat->blocks;
    while (cur_block) {
        free(cur_block->data);
        struct storage_block *old_block = cur_block;
        cur_block = cur_block->next;
        free(old_block);
    }

    if (!TlsFree(plat->tls_index)) {
        return false;
    }

    return true;
}

RBTK_NO_DISCARD void *
plat_rbtk_get_thread_storage(RBTK_THREAD_STORAGE_KEY *key)
{
    assert(key);

    PLAT_RBTK_THREAD_STORAGE_KEY *plat = key->plat;

    LPVOID *tls_storage = TlsGetValue(plat->tls_index);
    if (!tls_storage) {
        DWORD win32_error = GetLastError();
        if(win32_error) {
            SetLastError(win32_error);
            rbtk_signal_error(RBTK_ERROR_PLATFORM, NULL);
            return NULL;
        }

        struct storage_block *block = NULL;
        RBTK_MALLOC_OR_RETURN(&block, NULL,
            "failed to allocat storage for thread");

        void *memory = malloc(key->size);
        if (!memory) {
            rbtk_signal_error(RBTK_ERROR_OUT_OF_MEMORY,
                "failed to allocate storage for thread");
            return NULL;
        }
        memset(memory, 0x00, key->size);

        block->data = memory;
        block->next = NULL;

        if (!plat->blocks) {
            plat->blocks = block;
        } else {
            plat->blocks->next = block;
            plat->blocks = block;
        }

        TlsSetValue(plat->tls_index, memory);
        tls_storage = memory;
    }

    return tls_storage;
}

#endif /* defined(_WIN32) */
