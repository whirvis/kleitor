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

#include "error.h"

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#define _MULTI_THREADED
#include <pthread.h>

static bool initialized;
static __thread priv_rbtk_error_info *error_tls;

RBTK_PLATFORM RBTK_NO_DISCARD bool
plat_rbtk_error_init(void)
{
    if (initialized) {
    	return true;
    }

    /* nothing to do here */

    initialized = true;
    return true;
}

RBTK_PLATFORM RBTK_NO_DISCARD bool
plat_rbtk_error_terminate(void)
{
    if (!initialized) {
        return true;
    }

    /* nothing to do here */

    initialized = false;
    return true;
}

RBTK_PLATFORM RBTK_NO_DISCARD priv_rbtk_error_info *
plat_rbtk_get_error_info(bool allocate_if_missing, bool *allocated,
	RBTK_ERROR_CODE *error)
{
    assert(allocated);
    assert(error);

    *allocated = false;
    *error = RBTK_ERROR_NONE;

    priv_rbtk_error_info *info = error_tls;
    if (!info && allocate_if_missing) {
        info = malloc(sizeof(*info));
        if (!info) {
            *error = RBTK_ERROR_OUT_OF_MEMORY;
            return NULL;
        }

        error_tls = info;
        *allocated = true;
    }
    return info;
}

#endif /* defined(__linux__) */

