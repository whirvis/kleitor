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
#ifndef RBTK_ENGINE_PLATFORM_INPUT_H_
#define RBTK_ENGINE_PLATFORM_INPUT_H_

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "../input.h"
#include "../private/input.h"

#include "../../runtime/common.h"

typedef struct PLAT_RBTK_IO_DEVICE {
    union {
        struct {
            int glfw_index;
        } joystick;
    };
} PLAT_RBTK_IO_DEVICE;

RBTK_FORWARD_DECLARATION
typedef struct PLAT_RBTK_IO_FEATURE {
    union {
        struct {
            int glfw_key;
        } keyboard;
        struct {
            int glfw_button;
        } mouse;
        struct {
            int glfw_index;
        } joystick;
    };
} PLAT_RBTK_IO_FEATURE;

RBTK_PLATFORM RBTK_NO_DISCARD PLAT_RBTK_IO_DEVICE *
plat_rbtk_create_io_device(rbtk_io_device_type type);

RBTK_PLATFORM RBTK_NO_DISCARD bool
plat_rbtk_destroy_io_device(RBTK_IO_DEVICE *device);

RBTK_PLATFORM RBTK_NO_DISCARD bool
plat_rbtk_io_button_is_pressed(const RBTK_IO_DEVICE *device,
    const RBTK_IO_FEATURE *button);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* RBTK_ENGINE_PLATFORM_INPUT_H_ */
