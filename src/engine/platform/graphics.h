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
#ifndef RBTK_ENGINE_PLATFORM_GRAPHICS_H_
#define RBTK_ENGINE_PLATFORM_GRAPHICS_H_

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "../graphics.h"
#include "../private/graphics.h"

#include <stdbool.h>

#include "../../runtime/common.h"

RBTK_PLATFORM RBTK_FORWARD_DECLARATION
typedef struct PLAT_RBTK_MONITOR PLAT_RBTK_MONITOR;

RBTK_PLATFORM RBTK_FORWARD_DECLARATION
typedef struct PLAT_RBTK_WINDOW PLAT_RBTK_WINDOW;

RBTK_PLATFORM RBTK_FORWARD_DECLARATION
typedef struct PLAT_RBTK_GRAPHICS PLAT_RBTK_GRAPHICS;

RBTK_PLATFORM RBTK_FORWARD_DECLARATION
typedef struct PLAT_RBTK_SPRITE PLAT_RBTK_SPRITE;

RBTK_PLATFORM RBTK_NO_DISCARD bool
plat_rbtk_graphics_init(void);

RBTK_PLATFORM RBTK_NO_DISCARD bool
plat_rbtk_graphics_terminate(void);

RBTK_PLATFORM void
plat_rbtk_destroy_monitor(RBTK_MONITOR *monitor);

RBTK_PLATFORM RBTK_NO_DISCARD bool
plat_rbtk_create_window(RBTK_WINDOW *window,
    unsigned int width, unsigned int height);

RBTK_PLATFORM RBTK_NO_DISCARD bool
plat_rbtk_destroy_window(RBTK_WINDOW *window);

RBTK_PLATFORM RBTK_NO_DISCARD bool
plat_rbtk_show_window(RBTK_WINDOW *window);

RBTK_PLATFORM RBTK_NO_DISCARD bool
plat_rbtk_hide_window(RBTK_WINDOW *window);

RBTK_PLATFORM RBTK_NO_DISCARD bool
plat_rbtk_set_display_mode(RBTK_WINDOW *window,
	rbtk_display_mode mode);

RBTK_PLATFORM RBTK_NO_DISCARD bool
plat_rbtk_set_window_icon(RBTK_WINDOW *window,
    unsigned int width, unsigned int height, unsigned char *pixels);

RBTK_PLATFORM RBTK_NO_DISCARD bool
plat_rbtk_set_window_title(RBTK_WINDOW *window);

RBTK_PLATFORM RBTK_NO_DISCARD bool
plat_rbtk_get_window_size(const RBTK_WINDOW *window,
    unsigned int *width, unsigned int *height);

RBTK_PLATFORM RBTK_NO_DISCARD bool
plat_rbtk_set_window_size(RBTK_WINDOW *window,
    unsigned int width, unsigned int height);

RBTK_PLATFORM RBTK_NO_DISCARD bool
plat_rbtk_render_window_scene(const RBTK_WINDOW *window);

RBTK_PLATFORM RBTK_NO_DISCARD bool
plat_rbtk_create_scene(RBTK_GRAPHICS *scene,
    unsigned int width, unsigned int height);

RBTK_PLATFORM RBTK_NO_DISCARD bool
plat_rbtk_destroy_scene(RBTK_GRAPHICS *scene);

RBTK_PLATFORM void
plat_rbtk_clear_scene(RBTK_GRAPHICS *scene);

RBTK_PLATFORM PLAT_RBTK_SPRITE *
plat_rbtk_load_sprite(unsigned int width, unsigned int height,
    unsigned short channels, unsigned char *pixels);

RBTK_PLATFORM RBTK_NO_DISCARD bool
plat_rbtk_unload_sprite(RBTK_SPRITE *sprite);

RBTK_PLATFORM void
plat_rbtk_update_sprite_section(RBTK_SPRITE *sprite);

RBTK_PLATFORM void
plat_rbtk_draw_sprite(RBTK_GRAPHICS *graphics, RBTK_SPRITE *sprite,
    float x, float y, float z);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* RBTK_ENGINE_PLATFORM_GRAPHICS_H_ */
