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
#ifndef RBTK_ENGINE_PRIVATE_INPUT_H_
#define RBTK_ENGINE_PRIVATE_INPUT_H_

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "../input.h"

#include <stdbool.h>

#include "../../runtime/common.h"

RBTK_FORWARD_DECLARATION
typedef struct PLAT_RBTK_IO_DEVICE PLAT_RBTK_IO_DEVICE;

RBTK_FORWARD_DECLARATION
typedef struct PLAT_RBTK_IO_FEATURE PLAT_RBTK_IO_FEATURE;

typedef struct RBTK_IO_DEVICE {
    PLAT_RBTK_IO_DEVICE *plat;
    rbtk_io_device_type type;
    size_t max_features;
    size_t num_features;
    rbtk_io_feature_state **states;
} RBTK_IO_DEVICE;

/*
 * Note: The plat field is not an array of pointers. It is a pointer
 * to a single pointer. This is due to the fact everything must be a
 * constant when using static initialization.
 */
typedef struct RBTK_IO_FEATURE {
    PLAT_RBTK_IO_FEATURE **plat;
    const char *id;
    rbtk_io_feature_type type;
} RBTK_IO_FEATURE;

RBTK_NO_DISCARD bool
priv_rbtk_input_init(void);

RBTK_NO_DISCARD bool
priv_rbtk_input_terminate(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* RBTK_ENGINE_PRIVATE_INPUT_H_ */
