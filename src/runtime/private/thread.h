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
#ifndef RBTK_PRIVATE_THREAD_H_
#define RBTK_PRIVATE_THREAD_H_

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "../thread.h"

#include "../common.h"

RBTK_PLATFORM RBTK_FORWARD_DECLARATION
typedef struct PLAT_RBTK_THREAD PLAT_RBTK_THREAD;

RBTK_PRIVATE
typedef struct RBTK_THREAD {
    size_t id;
    char name[RBTK_THREAD_NAME_MAX_LENGTH];
    rbtk_thread_entrypoint entrypoint;
    void *params;
    rbtk_thread_priority priority;
    bool daemon;
    bool running;
    bool stopped;
    bool interrupted;
    PLAT_RBTK_THREAD *plat;
    RBTK_THREAD *prev;
    RBTK_THREAD *next;
} RBTK_THREAD;

RBTK_PLATFORM RBTK_FORWARD_DECLARATION
typedef struct PLAT_RBTK_THREAD_STORAGE_KEY PLAT_RBTK_THREAD_STORAGE_KEY;

RBTK_PRIVATE
typedef struct RBTK_THREAD_STORAGE_KEY {
    size_t size;
    PLAT_RBTK_THREAD_STORAGE_KEY *plat;
    RBTK_THREAD_STORAGE_KEY *prev;
    RBTK_THREAD_STORAGE_KEY *next;
} RBTK_THREAD_STORAGE_KEY;

RBTK_PRIVATE RBTK_NO_DISCARD bool
priv_rbtk_thread_init(void);

RBTK_PRIVATE RBTK_NO_DISCARD bool
priv_rbtk_thread_terminate(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* RBTK_PRIVATE_THREAD_H_ */
