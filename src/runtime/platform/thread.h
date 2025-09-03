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
#ifndef RBTK_PLATFORM_THREAD_H_
#define RBTK_PLATFORM_THREAD_H_

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "../thread.h"
#include "../private/thread.h"

#include <stdbool.h>

#include "../common.h"

RBTK_PLATFORM RBTK_FORWARD_DECLARATION
typedef struct PLAT_RBTK_THREAD PLAT_RBTK_THREAD;

RBTK_PLATFORM RBTK_FORWARD_DECLARATION
typedef struct PLAT_RBTK_LOCK PLAT_RBTK_LOCK;

RBTK_PLATFORM RBTK_FORWARD_DECLARATION
typedef struct PLAT_RBTK_RW_LOCK PLAT_RBTK_RW_LOCK;

RBTK_PLATFORM RBTK_NO_DISCARD bool
plat_rbtk_thread_init(void);

RBTK_PLATFORM RBTK_NO_DISCARD bool
plat_rbtk_thread_terminate(void);

RBTK_PLATFORM RBTK_NO_DISCARD RBTK_THREAD *
plat_rbtk_current_thread(void);

RBTK_PLATFORM RBTK_NO_DISCARD bool
plat_rbtk_create_thread(RBTK_THREAD *thread);

RBTK_PLATFORM RBTK_NO_DISCARD bool
plat_rbtk_destroy_thread(RBTK_THREAD *thread);

RBTK_PLATFORM bool
plat_rbtk_apply_thread_priority(RBTK_THREAD *thread);

RBTK_PLATFORM RBTK_NO_DISCARD bool
plat_rbtk_start_thread(RBTK_THREAD *thread);

RBTK_PLATFORM RBTK_NO_DISCARD bool
plat_rbtk_stop_thread(RBTK_THREAD *thread);

RBTK_PLATFORM RBTK_NO_DISCARD bool
plat_rbtk_create_thread_storage(RBTK_THREAD_STORAGE_KEY *key);

RBTK_PLATFORM RBTK_NO_DISCARD bool
plat_rbtk_destroy_thread_storage(RBTK_THREAD_STORAGE_KEY *key);

RBTK_PLATFORM RBTK_NO_DISCARD void *
plat_rbtk_get_thread_storage(RBTK_THREAD_STORAGE_KEY *key);

RBTK_PLATFORM void
plat_rbtk_yield_thread(RBTK_THREAD *thread);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* RBTK_PLATFORM_THREAD_H_ */
