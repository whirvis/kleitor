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
#ifndef RBTK_ENGINE_PLATFORM_ENGINE_H_
#define RBTK_ENGINE_PLATFORM_ENGINE_H_

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "../engine.h"
#include "../private/engine.h"

#include <stdbool.h>

#include "../../runtime/common.h"

RBTK_PLATFORM bool
plat_rbtk_engine_pre_init(void);

RBTK_PLATFORM bool
plat_rbtk_engine_post_init(void);

RBTK_PLATFORM bool
plat_rbtk_engine_pre_terminate(void);

RBTK_PLATFORM bool
plat_rbtk_engine_post_terminate(void);

RBTK_PLATFORM void
plat_rbtk_engine_pre_update(void);

RBTK_PLATFORM void
plat_rbtk_engine_post_update(void);

RBTK_PLATFORM void
plat_rbtk_engine_pre_render(void);

RBTK_PLATFORM void
plat_rbtk_engine_post_render(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* RBTK_ENGINE_PLATFORM_ENGINE_H_ */
