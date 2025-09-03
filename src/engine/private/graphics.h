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
#ifndef RBTK_ENGINE_PRIVATE_GRAPHICS_H_
#define RBTK_ENGINE_PRIVATE_GRAPHICS_H_

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "../graphics.h"

#include <stdbool.h>

#include "../../libraries/cglm_no_io.h"

#include "../../runtime/common.h"

RBTK_PLATFORM RBTK_FORWARD_DECLARATION
typedef struct PLAT_RBTK_MONITOR PLAT_RBTK_MONITOR;

RBTK_PLATFORM RBTK_FORWARD_DECLARATION
typedef struct PLAT_RBTK_WINDOW PLAT_RBTK_WINDOW;

RBTK_PLATFORM RBTK_FORWARD_DECLARATION
typedef struct PLAT_RBTK_GRAPHICS PLAT_RBTK_GRAPHICS;

RBTK_PLATFORM RBTK_FORWARD_DECLARATION
typedef struct PLAT_RBTK_SPRITE PLAT_RBTK_SPRITE;

RBTK_PLATFORM RBTK_FORWARD_DECLARATION
typedef struct PLAT_RBTK_SPRITESHEET PLAT_RBTK_SPRITESHEET;

typedef struct RBTK_MONITOR {
    PLAT_RBTK_MONITOR *plat;
} RBTK_MONITOR;

typedef struct RBTK_WINDOW {
    PLAT_RBTK_WINDOW *plat;
    rbtk_window_caps caps;
    rbtk_display_mode display_mode;
    unsigned int width;
    unsigned int height;
    bool visible;
    bool focused;
    bool should_close;
    char *title;
    RBTK_GRAPHICS *scene;
    mat4 scene_model;
    mat4 scene_view;
    mat4 scene_proj;
} RBTK_WINDOW;

typedef struct RBTK_PROJECTION {
    rbtk_projection_specs specs;
    mat4 matrix;
} RBTK_PROJECTION;

typedef struct RBTK_GRAPHICS {
    PLAT_RBTK_GRAPHICS *plat;
    const RBTK_PROJECTION *proj;
    unsigned int width;
    unsigned int height;
    RBTK_CAMERA *camera;
    RBTK_SPRITE *sprite;
    RBTK_WINDOW *windows[RBTK_MAX_WINDOW_COUNT];
} RBTK_GRAPHICS;

typedef struct RBTK_CAMERA {
    vec3 pos;
} RBTK_CAMERA;

typedef struct RBTK_SPRITE {
    PLAT_RBTK_SPRITE *plat;
    RBTK_GRAPHICS *scene;
    unsigned int width;
    unsigned int height;
    struct {
        bool vertically;
        bool horizontally;
    } flipped;
    struct {
        unsigned int x;
        unsigned int y;
        unsigned int width;
        unsigned int height;
    } section;
    struct {
        float x;
        float y;
        float z;
    } offset;
    struct {
        float x;
        float y;
        float z;
    } rotation;
    struct {
        float x;
        float y;
        float z;
    } scale;
    struct {
        float red;
        float green;
        float blue;
        float alpha;
    } color;
    mat4 model;
} RBTK_SPRITE;

typedef struct RBTK_SPRITE_ANIME {
    size_t max_frames;
    size_t num_frames;
    RBTK_SPRITE **frames;
    long double *durations;
    long double timer;
    bool loop;
    bool ping_pong;
    bool backwards;
    bool finished;
    size_t current_frame;
    struct {
        float x;
        float y;
        float z;
    } offset;
} RBTK_SPRITE_ANIME;

RBTK_PRIVATE RBTK_NO_DISCARD bool
priv_rbtk_graphics_init(void);

RBTK_PRIVATE RBTK_NO_DISCARD bool
priv_rbtk_graphics_terminate(void);

RBTK_PRIVATE RBTK_NO_DISCARD bool
priv_rbtk_add_monitor(RBTK_MONITOR *plat);

RBTK_PRIVATE void
priv_rbtk_destroy_monitor(RBTK_MONITOR *monitor);

RBTK_PRIVATE RBTK_NO_DISCARD bool
priv_rbtk_add_window(RBTK_WINDOW *plat);

RBTK_PRIVATE RBTK_NO_DISCARD RBTK_WINDOW *
priv_rbtk_create_window(unsigned int width, unsigned int height);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* RBTK_ENGINE_PRIVATE_GRAPHICS_H_ */
