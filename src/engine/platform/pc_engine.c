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
#if defined(_WIN32) || defined(__linux__)

#include "engine.h"

#include "../../runtime/error.h"

/* implemented in opengl_graphics.c */
void
plat_rbtk_update_glfw(void);

static bool pre_initialized;
static bool post_initialized;

RBTK_PLATFORM bool
plat_rbtk_engine_pre_init(void)
{
    if (pre_initialized) {
        return true;
    }

    /* nothing to do here yet */

    pre_initialized = true;
    return true;
}

RBTK_PLATFORM bool
plat_rbtk_engine_post_init(void)
{
    if (post_initialized) {
        return true;
    }

    if (!pre_initialized) {
        rbtk_signal_error(RBTK_ERROR_ILLEGAL_STATE,
            "engine must be pre-initialized first");
        return false;
    }

    /* nothing to do here yet */

    post_initialized = true;
    return true;
}

RBTK_PLATFORM bool
plat_rbtk_engine_pre_terminate(void)
{
    if (!pre_initialized) {
        return true;
    }

    /* nothing to do here yet */

    pre_initialized = false;
    return true;
}

RBTK_PLATFORM bool
plat_rbtk_engine_post_terminate(void)
{
    if (!post_initialized) {
        return true;
    }

    if (pre_initialized) {
        rbtk_signal_error(RBTK_ERROR_ILLEGAL_STATE,
            "engine must be pre-terminated first");
        return false;
    }

    /* nothing to do here yet */

    post_initialized = false;
    return true;
}

RBTK_PLATFORM void
plat_rbtk_engine_pre_update(void)
{
    plat_rbtk_update_glfw();
}

RBTK_PLATFORM void
plat_rbtk_engine_post_update(void)
{
    /* nothing to do here yet */
}

RBTK_PLATFORM void
plat_rbtk_engine_pre_render(void)
{
    /* nothing to do here yet */
}

RBTK_PLATFORM void
plat_rbtk_engine_post_render(void)
{
    /* nothing to do here yet */
}

#endif /* defined (_WIN32) || defined(__linux__) */
