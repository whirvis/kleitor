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
#include "thread.h"
#include "./private/thread.h"
#include "./platform/thread.h"

#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "error.h"

#define REQUIRE_INITIALIZED_OR_RETURN(_value)       \
    if (!initialized) {                             \
        rbtk_signal_error(RBTK_ERROR_ILLEGAL_STATE, \
            "thread module not initialized");       \
        return (_value);                            \
    }

#define USE_GIVEN_THREAD_OR_CURRENT(_thread)    \
    do {                                        \
        if(!(*_thread)) {                       \
            (*_thread) = rbtk_current_thread(); \
        }                                       \
    } while(0)

static RBTK_THREAD main_thread = {
    .id = 0,
    .name = "main",
    .entrypoint = NULL,
    .params = NULL,
    .priority = RBTK_THREAD_PRIORITY_NORMAL,
    .daemon = false,
    .running = true,
    .stopped = false,
    .interrupted = false,
    .plat = NULL,
};

static RBTK_THREAD *threads_head;
static RBTK_THREAD *threads_tail;
static RBTK_THREAD_STORAGE_KEY *keys_head;
static RBTK_THREAD_STORAGE_KEY *keys_tail;
static size_t next_thread_id;
static bool initialized;

RBTK_NO_DISCARD bool
priv_rbtk_thread_init(void)
{
    if (initialized) {
        return true;
    }

    if (!plat_rbtk_thread_init()) {
        return false;
    }

    threads_head = NULL;
    threads_tail = NULL;
    keys_head = NULL;
    keys_tail = NULL;
    next_thread_id = 1;

    initialized = true;
    return true;
}

RBTK_NO_DISCARD bool
priv_rbtk_thread_terminate(void)
{
    if (!initialized) {
        return true;
    }

    if (!plat_rbtk_thread_terminate()) {
        return false;
    }

    bool destroyed_all_threads = true;
    RBTK_THREAD *cur_thread = threads_head;
    while (cur_thread) {
        if (!cur_thread->daemon) {
            rbtk_join_thread(cur_thread);
        }
        if (!plat_rbtk_destroy_thread(cur_thread)) {
            destroyed_all_threads = false;
        }
        RBTK_THREAD *prev = cur_thread;
        cur_thread = cur_thread->next;
        free(prev);
    }

    bool destroyed_all_keys = true;
    RBTK_THREAD_STORAGE_KEY *cur_key = keys_head;
    while (cur_key) {
        if (!plat_rbtk_destroy_thread_storage(cur_key)) {
            destroyed_all_keys = false;
        }
        RBTK_THREAD_STORAGE_KEY *prev = cur_key;
        cur_key = cur_key->next;
        free(prev);
    }

    threads_head = NULL;
    threads_tail = NULL;
    keys_head = NULL;
    keys_tail = NULL;
    next_thread_id = 1;

    initialized = false;
    return destroyed_all_threads && destroyed_all_keys;
}

RBTK_NO_DISCARD RBTK_THREAD *
rbtk_current_thread(void)
{
    REQUIRE_INITIALIZED_OR_RETURN(NULL);
    return plat_rbtk_current_thread();
}

RBTK_NO_DISCARD RBTK_THREAD *
rbtk_create_thread(const char *name,
    rbtk_thread_entrypoint entrypoint,
    void *params)
{
    assert(name);
    assert(entrypoint);
    REQUIRE_INITIALIZED_OR_RETURN(NULL);

    RBTK_THREAD *thread;
    RBTK_MALLOC_OR_RETURN(&thread, false,
        "failed to allocate memory for thread");

    thread->id = next_thread_id++;
    strncpy(thread->name, name, RBTK_THREAD_NAME_MAX_LENGTH);
    thread->name[RBTK_THREAD_NAME_MAX_LENGTH - 1] = '\0';
    thread->entrypoint = entrypoint;
    thread->params = params;
    thread->priority = RBTK_THREAD_PRIORITY_NORMAL;
    thread->daemon = false;
    thread->running = false;
    thread->stopped = false;
    thread->interrupted = false;
    thread->next = NULL;
    thread->prev = NULL;

    if (!plat_rbtk_create_thread(thread)) {
        return false;
    }

    RBTK_DLL_PUSH(threads_head, threads_tail, thread);
    return thread;
}

bool
rbtk_destroy_thread(RBTK_THREAD *thread)
{
    assert(thread);
    RBTK_DLL_REMOVE(threads_head, threads_tail, thread);
    bool plat_destroyed = plat_rbtk_destroy_thread(thread);
    free(thread);
    return plat_destroyed;
}

RBTK_NO_DISCARD size_t
rbtk_get_thread_id(const RBTK_THREAD *thread)
{
    USE_GIVEN_THREAD_OR_CURRENT(&thread);
    return thread->id;
}

RBTK_NO_DISCARD const char *
rbtk_get_thread_name(const RBTK_THREAD *thread)
{
    USE_GIVEN_THREAD_OR_CURRENT(&thread);
    return thread->name;
}

void
rbtk_set_thread_name(RBTK_THREAD *thread, const char *name)
{
    assert(name);
    USE_GIVEN_THREAD_OR_CURRENT(&thread);
    strncpy(thread->name, name, RBTK_THREAD_NAME_MAX_LENGTH);
    thread->name[RBTK_THREAD_NAME_MAX_LENGTH - 1] = '\0';
}

RBTK_NO_DISCARD bool
rbtk_thread_is_daemon(RBTK_THREAD *thread)
{
    USE_GIVEN_THREAD_OR_CURRENT(&thread);
    return thread->daemon;
}

void
rbtk_thread_set_daemon(RBTK_THREAD *thread, bool daemon)
{
    USE_GIVEN_THREAD_OR_CURRENT(&thread);
    thread->daemon = daemon;
}

RBTK_NO_DISCARD rbtk_thread_priority
rbtk_get_thread_priority(RBTK_THREAD *thread)
{
    USE_GIVEN_THREAD_OR_CURRENT(&thread);
    return thread->priority;
}

bool
rbtk_set_thread_priority(RBTK_THREAD *thread, rbtk_thread_priority priority)
{
    USE_GIVEN_THREAD_OR_CURRENT(&thread);
    thread->priority = priority;
    return plat_rbtk_apply_thread_priority(thread);
}

RBTK_NO_DISCARD bool
rbtk_thread_is_alive(RBTK_THREAD *thread)
{
    assert(thread);
    return thread->running;
}

bool
rbtk_start_thread(RBTK_THREAD *thread)
{
    assert(thread);
    if (thread->running) {
        rbtk_signal_error(RBTK_ERROR_ILLEGAL_STATE,
            "thread already running");
        return false;
    }
    else if (thread->stopped) {
        rbtk_signal_error(RBTK_ERROR_ILLEGAL_STATE,
            "thread already stopped");
        return false;
    }

    /*
     * Set the running flag to true as soon as possible just to be safe.
     * However, if the platform indicates it failed to start the thread
     * then set the flag back to false so the caller can try starting it
     * at another time if they desire.
     */
    thread->running = true;
    bool plat_started = plat_rbtk_start_thread(thread);
    if (!plat_started) {
        thread->running = false;
    }

    return plat_started;
}

bool
rbtk_stop_thread(RBTK_THREAD *thread)
{
    assert(thread);
    if (!thread->running || thread->stopped) {
        return true; /* nothing to do */
    }

    /*
     * Set the stopped flag to true as soon as possible just to be safe.
     * However, if the platform indicates it failed to stop the thread
     * then set the flag back to false so the caller can try stopping it
     * at another time if they desire.
     */
    thread->stopped = true;
    bool plat_stopped = plat_rbtk_stop_thread(thread);
    if (!plat_stopped) {
        thread->stopped = false;
    }
    else {
        thread->interrupted = false;
        thread->running = false;
    }

    return plat_stopped;
}

RBTK_NO_DISCARD bool
rbtk_thread_interrupted(RBTK_THREAD *thread, bool clear_flag)
{
    USE_GIVEN_THREAD_OR_CURRENT(&thread);
    bool interrupted = thread->interrupted;
    if (interrupted && clear_flag) {
        thread->interrupted = false;
    }
    return interrupted;
}

void
rbtk_interrupt_thread(RBTK_THREAD *thread)
{
    USE_GIVEN_THREAD_OR_CURRENT(&thread);
    if (thread->running) {
        thread->interrupted = true;
    }
}

void
rbtk_yield_thread(RBTK_THREAD *thread)
{
    USE_GIVEN_THREAD_OR_CURRENT(&thread);
    plat_rbtk_yield_thread(thread);
}

void
rbtk_join_thread(RBTK_THREAD *thread)
{
    assert(thread);
    assert(thread != rbtk_current_thread());
    while (thread->running);
}

bool
rbtk_join_thread_within(RBTK_THREAD *thread, rbtk_time_unit unit,
    long double timeout)
{
    assert(thread);
    assert(thread != rbtk_current_thread());
    if (!thread->running) {
        return true;
    }

    long double begin_wait = rbtk_time(unit);
    while (rbtk_time(unit) - begin_wait < timeout) {
        if (!thread->running) {
            return true;
        }
    }

    return false;
}

RBTK_NO_DISCARD RBTK_THREAD_STORAGE_KEY *
rbtk_create_thread_storage(size_t size)
{
    assert(size > 0);
    REQUIRE_INITIALIZED_OR_RETURN(NULL);

    if (rbtk_current_thread() != RBTK_MAIN_THREAD) {
        rbtk_signal_error(RBTK_ERROR_ILLEGAL_STATE,
            "only the main thread can create thread-local storage");
        return NULL;
    }

    RBTK_THREAD_STORAGE_KEY *key;
    RBTK_MALLOC_OR_RETURN(&key, NULL,
        "failed to allocate thread storage key");

    key->size = size;
    bool plat_created = plat_rbtk_create_thread_storage(key);
    if (!plat_created) {
        free(key);
        return NULL;
    }

    RBTK_DLL_PUSH(keys_head, keys_tail, key);
    return key;
}

bool
rbtk_destroy_thread_storage(RBTK_THREAD_STORAGE_KEY *key)
{
    assert(key);
    RBTK_DLL_REMOVE(keys_head, keys_tail, key);
    bool plat_destroyed = plat_rbtk_destroy_thread_storage(key);
    free(key);
    return plat_destroyed;
}

RBTK_NO_DISCARD void *
rbtk_get_thread_storage(RBTK_THREAD_STORAGE_KEY *key)
{
    assert(key);
    return plat_rbtk_get_thread_storage(key);
}

RBTK_THREAD *const RBTK_MAIN_THREAD = &main_thread;
