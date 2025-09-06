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
#include "graphics.h"
#include "./private/graphics.h"
#include "./platform/graphics.h"

#include <assert.h>
#include <math.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "../libraries/cglm_no_io.h"
#include "../libraries/stb_image.h"

#include "../runtime/asset.h"
#include "../runtime/common.h"
#include "../runtime/time.h"

#define REQUIRE_INITIALIZED_OR_RETURN(_value)       \
    if (!initialized) {                             \
        rbtk_signal_error(RBTK_ERROR_ILLEGAL_STATE, \
            "grpahics module not initialized");     \
        return (_value);                            \
    }

static size_t monitor_count;
static RBTK_MONITOR *monitors[RBTK_MAX_MONITOR_COUNT];
static size_t window_count;
static RBTK_WINDOW *windows[RBTK_MAX_WINDOW_COUNT];
static bool initialized;

RBTK_PRIVATE RBTK_NO_DISCARD bool
priv_rbtk_graphics_init(void)
{
    if (initialized) {
        return true;
    }

    monitor_count = 0;
    for (size_t i = 0; i < RBTK_MAX_MONITOR_COUNT; i++) {
        monitors[i] = NULL;
    }

    window_count = 0;
    for (size_t i = 0; i < RBTK_MAX_WINDOW_COUNT; i++) {
        windows[i] = NULL;
    }

    if (!plat_rbtk_graphics_init()) {
        return false;
    }

    initialized = true;
    return true;
}

RBTK_PRIVATE RBTK_NO_DISCARD bool
priv_rbtk_graphics_terminate(void)
{
    if (!initialized) {
        return true;
    }

    for (size_t i = 0; i < monitor_count; i++) {
        priv_rbtk_destroy_monitor(monitors[i]);
    }
    monitor_count = 0;

    for (size_t i = 0; i < window_count; i++) {
        rbtk_destroy_window(windows[i]);
    }
    window_count = 0;

    if (!plat_rbtk_graphics_terminate()) {
        return false;
    }

    initialized = false;
    return true;
}

RBTK_PRIVATE RBTK_NO_DISCARD bool
priv_rbtk_add_monitor(RBTK_MONITOR *monitor)
{
    assert(monitor);

    if (monitor_count >= RBTK_MAX_MONITOR_COUNT) {
        rbtk_signal_error(RBTK_ERROR_OUT_OF_MEMORY,
            "max monitor count reached");
        return false;
    }

    monitors[monitor_count] = monitor;
    monitor_count += 1;

    return true;
}

RBTK_PRIVATE void
priv_rbtk_destroy_monitor(RBTK_MONITOR *monitor) {
    if (!monitor) {
        return;
    }

    for (size_t i = 0; i < RBTK_MAX_MONITOR_COUNT; i++) {
        if (monitors[i] == monitor) {
            monitors[i] = NULL;
            break; /* there should only be one instance */
        }
    }

    plat_rbtk_destroy_monitor(monitor);
}

RBTK_PRIVATE RBTK_NO_DISCARD bool
priv_rbtk_add_window(RBTK_WINDOW *window)
{
    assert(window);

    if (window_count >= RBTK_MAX_WINDOW_COUNT) {
        rbtk_signal_error(RBTK_ERROR_OUT_OF_MEMORY,
            "max window count reached");
        return false;
    }

    windows[window_count] = window;
    window_count += 1;

    return true;
}

RBTK_NO_DISCARD RBTK_MONITORS
rbtk_get_monitors(size_t *count)
{
    assert(count);
    *count = 0;
    REQUIRE_INITIALIZED_OR_RETURN(NULL);
    *count = monitor_count;
    return monitors;
}

RBTK_NO_DISCARD RBTK_MONITOR *
rbtk_get_monitor(size_t index)
{
    REQUIRE_INITIALIZED_OR_RETURN(NULL);
    if (index >= monitor_count) {
        rbtk_signal_error(RBTK_ERROR_OUT_OF_BOUNDS,
            "index %d exceeds monitor array of size %d",
            index, monitor_count);
        return NULL;
    }
    return monitors[index];
}

RBTK_NO_DISCARD RBTK_WINDOWS
rbtk_get_windows(size_t *count)
{
    assert(count);
    *count = 0;
    REQUIRE_INITIALIZED_OR_RETURN(NULL);
    *count = window_count;
    return windows;
}

RBTK_NO_DISCARD RBTK_WINDOW *
rbtk_get_window(size_t index)
{
    REQUIRE_INITIALIZED_OR_RETURN(NULL);
    if (index >= window_count) {
        rbtk_signal_error(RBTK_ERROR_OUT_OF_BOUNDS,
            "index %d exceeds window array of size %d",
            index, window_count);
        return NULL;
    }
    return windows[index];
}

RBTK_PRIVATE RBTK_NO_DISCARD RBTK_WINDOW *
priv_rbtk_create_window(unsigned int width, unsigned int height)
{
    RBTK_WINDOW *window = NULL;
    RBTK_MALLOC_OR_RETURN(&window, NULL,
        "could not allocate memory for window");

    window->plat = NULL;
    RBTK_ZERO_MEMORY(&window->caps);
    window->display_mode = RBTK_WINDOWED;
    window->width = width;
    window->height = height;

    window->visible = false;
    window->focused = false;
    window->should_close = false;
    window->title = NULL;
    window->scene = NULL;

    if (!plat_rbtk_create_window(window, width, height)) {
        free(window);
        rbtk_suggest_error(RBTK_ERROR_PLATFORM,
            "failed to create window for current platform");
        return NULL;
    }

    if (!priv_rbtk_add_window(window)) {
        free(window);
        rbtk_suggest_error(RBTK_ERROR_OUT_OF_MEMORY,
            "failed to regster newly created window");
        return NULL;
    }

    return window;
}

RBTK_NO_DISCARD RBTK_WINDOW *
rbtk_create_window(unsigned int width, unsigned int height,
    const char *title)
{
    REQUIRE_INITIALIZED_OR_RETURN(NULL);
    RBTK_WINDOW *window = priv_rbtk_create_window(width, height);
    if (window) {
        rbtk_set_window_title(window, title);
    }
    return window;
}

bool
rbtk_destroy_window(RBTK_WINDOW *window)
{
    if (!window) {
        return true;
    }

    if (window == windows[0]) {
        rbtk_signal_error(RBTK_ERROR_ILLEGAL_ARGUMENT,
            "cannot destroy the primary window");
        return false;
    }

    /*
     * Since this window has been destroyed, free up the slot in the
     * array. This prevents the array from being filled with dangling
     * pointers. If the array is filled with these dangling pointers,
     * then it will eventually become impossible possible to make more
     * windows even if previous windows have been destroyed.
     */
    for (size_t i = 0; i < RBTK_MAX_WINDOW_COUNT; i++) {
        if (windows[i] == window) {
            windows[i] = NULL;
            break; /* there should only be one instance */
        }
    }

    bool success = plat_rbtk_destroy_window(window);
    free(window); /* we no longer need this */
    return success;
}

RBTK_NO_DISCARD const rbtk_window_caps *
rbtk_get_window_caps(const RBTK_WINDOW *window)
{
    assert(window);
    return &window->caps;
}

RBTK_NO_DISCARD bool
rbtk_window_is_visible(RBTK_WINDOW *window)
{
    assert(window);
    return window->visible;
}

bool
rbtk_show_window(RBTK_WINDOW *window)
{
    assert(window);
    if (window->visible) {
        return true;
    }

    if (!plat_rbtk_show_window(window)) {
        rbtk_suggest_error(RBTK_ERROR_PLATFORM,
            "cannot show window on current platform");
        return false;
    }

    window->visible = true;
    return true;
}

bool
rbtk_hide_window(RBTK_WINDOW *window)
{
    assert(window);
    if (!window->visible) {
        return true;
    }

    if (!plat_rbtk_hide_window(window)) {
        rbtk_suggest_error(RBTK_ERROR_PLATFORM,
            "cannot hide window on current platform");
        return false;
    }

    window->visible = false;
    return true;
}

bool
rbtk_set_display_mode(RBTK_WINDOW *window, rbtk_display_mode mode)
{
    assert(window);
    if (window->display_mode == mode) {
        return true;
    }

    if (!plat_rbtk_set_display_mode(window, mode)) {
        rbtk_suggest_error(RBTK_ERROR_PLATFORM,
            "current platform does not support display mode %d", mode);
        return false;
    }

    window->display_mode = mode;
    return true;
}

RBTK_NO_DISCARD rbtk_display_mode
rbtk_get_display_mode(RBTK_WINDOW *window)
{
    assert(window);
    return window->display_mode;
}

RBTK_NO_DISCARD bool
rbtk_window_should_close(RBTK_WINDOW *window)
{
    assert(window);
    return window->should_close;
}

RBTK_NO_DISCARD bool
rbtk_any_windows_should_close(void) {
    for (size_t i = 0; i < window_count; i++) {
        RBTK_WINDOW *window = windows[i];
        if (window->should_close) {
            return true;
        }
    }
    return false;
}

bool
rbtk_set_window_icon(RBTK_WINDOW *window, RBTK_ASSET *asset)
{
    assert(window);
    if (!asset) {
        return plat_rbtk_set_window_icon(window, 0, 0, NULL);
    }

    RBTK_IN_STREAM *in = rbtk_open_asset_in_stream(asset);
    size_t buffer_size = 0;
    unsigned char *buffer = rbtk_buffer_remaining(in, &buffer_size);
    rbtk_close_in_stream(in);
    if (!buffer) {
        return false;
    }

    int width, height, channels;
    stbi_uc *img = stbi_load_from_memory(buffer, (int) buffer_size,
        &width, &height, &channels, 0);
    bool success = plat_rbtk_set_window_icon(window, width, height, img);
    stbi_image_free(img);

    return success;
}

void
rbtk_set_window_should_close(RBTK_WINDOW *window, bool close)
{
    assert(window);
    window->should_close = close;
}

RBTK_NO_DISCARD const char *
rbtk_get_window_title(const RBTK_WINDOW *window)
{
    assert(window);
    return window->title;
}

bool
rbtk_set_window_title(RBTK_WINDOW *window, const char *title)
{
    assert(window);
    if (!title) {
        /* default to an empty string */
        title = "";
    }

    free(window->title);

    size_t title_len = strlen(title) + 1;
    window->title = malloc(title_len);
    if (!window->title) {
        rbtk_signal_error(RBTK_ERROR_OUT_OF_MEMORY,
            "could not allocate memory for window title");
        return false;
    }

    strncpy(window->title, title, title_len);
    window->title[title_len - 1] = '\0';

    return plat_rbtk_set_window_title(window);
}

bool
rbtk_get_window_size(const RBTK_WINDOW *window,
    unsigned int *width, unsigned int *height)
{
    assert(window);
    assert(width || height);

    unsigned int current_width = 0;
    unsigned int current_height = 0;
    bool success = plat_rbtk_get_window_size(window,
        &current_width, &current_height);

    if (width) {
        *width = current_width;
    }
    if (height) {
        *height = current_height;
    }

    return success;
}

bool
rbtk_set_window_size(RBTK_WINDOW *window,
    unsigned int width, unsigned int height)
{
    assert(window);
    if (width == 0 && height == 0) {
        return true; /* nothing to change */
    }

    unsigned int current_width = 0;
    unsigned int current_height = 0;
    bool retrieved_size = rbtk_get_window_size(window,
        &current_height, &current_height);
    if (!retrieved_size && (width == 0 || height == 0)) {
        return false;
    }

    /* use current width or height if directed */
    width = (width != 0) ? width : current_width;
    height = (height != 0) ? height : current_height;

    return plat_rbtk_set_window_size(window, width, height);
}

RBTK_GRAPHICS *
rbtk_get_window_scene(const RBTK_WINDOW *window)
{
    assert(window);
    return window->scene;
}

void
rbtk_bind_scene_to_window(RBTK_WINDOW *window, RBTK_GRAPHICS *scene)
{
    assert(window);
    if (window->scene == scene) {
        return; /* nothing to do */
    }

    /*
     * Update the graphics scene so it no longer sees itself as bound to
     * this window. If this is not neglected, it will not be possible to
     * destroy the graphics scene when requested.
     */
    if (window->scene) {
        RBTK_GRAPHICS *current = window->scene;
        for (size_t i = 0; i < RBTK_MAX_WINDOW_COUNT; i++) {
            if (current->windows[i] == window) {
                current->windows[i] = NULL;
                break; /* there should only be one instance */
            }
        }
    }

    /* a NULL scene means we're unbinding */
    if (!scene) {
        window->scene = NULL;
        return; /* nothing to do */
    }

    /*
     * After binding to the scene, we must notify it that we have bound to
     * it. This ensures it will not be destroyed until this window and all
     * other windows have been unbound from it.
     */
    window->scene = scene;
    for (size_t i = 0; i < RBTK_MAX_WINDOW_COUNT; i++) {
        if (!scene->windows[i]) {
            scene->windows[i] = window;
            break; /* only use the first empty slot */
        }
    }

    /* always initialize to identity just to be safe */
    glm_mat4_identity(window->scene_model);
    glm_mat4_identity(window->scene_proj);
    glm_mat4_identity(window->scene_view);

    vec3 model_translate = { 0, 0, 0 };

    vec3 camera_pos = { 0, 0, 1 };
    vec3 camera_target = { 0, 0, 0 };
    vec3 camera_up = { 0, 1, 0 };

    /*
 * Now that we have all the necessary information, we can calculate the
 * model view projection matrices, which will determine the final result
 * of rendering this scene to the window.
 */
    glm_translate(window->scene_model, model_translate);
    glm_lookat(camera_pos, camera_target, camera_up, window->scene_view);
    glm_ortho(0, (float) scene->width, (float) scene->height, 0.0f,
        (float) scene->proj->specs.near, (float) scene->proj->specs.far,
        window->scene_proj);
}

bool
rbtk_render_window_scene(const RBTK_WINDOW *window)
{
    assert(window);
    if (!window->scene) {
        return false;
    }

    if (!plat_rbtk_render_window_scene(window)) {
        rbtk_suggest_error(RBTK_ERROR_PLATFORM,
            "error rendering graphics scene to glfw_window");
        return false;
    }

    return true;
}

void
rbtk_clear_window_scene(const RBTK_WINDOW *window) {
    assert(window);
    if (window->scene) {
        rbtk_clear_scene(window->scene);
    }
}

RBTK_NO_DISCARD RBTK_PROJECTION *
rbtk_create_ortho_projection(float left, float right, float bottom, float top,
    float near, float far)
{
    RBTK_PROJECTION *proj = NULL;
    RBTK_MALLOC_OR_RETURN(&proj, NULL,
        "could not allocate memory for orthograhic projection");

    rbtk_projection_specs specs = {
        .type = RBTK_PROJECTION_ORTHO,
        .ortho = {
            .left = left,
            .right = right,
            .top = top,
            .bottom = bottom,
        },
        .near = near,
        .far = far,
        .width = (float) fabs(right - left),
        .height = (float) fabs(bottom - top)
    };

    /*
     * Note: The top and bottom of this orthographic matrix are flipped
     * here on purpose!
     *
     * Not doing this results in the output being upside down. This only
     * seems to occur with orthographic matrices when rendering to frame
     * buffers. This does not seem to occur with perspective projections
     * (where multiplying the FOV by -1 would flip it vertically.)
     */
    proj->specs = specs;
    glm_ortho(left, right, top, bottom, near, far, proj->matrix);
    return proj;
}

RBTK_NO_DISCARD RBTK_PROJECTION *
rbtk_create_persp_projection(float fov, float width, float height,
    float near, float far)
{
    RBTK_PROJECTION *proj = NULL;
    RBTK_MALLOC_OR_RETURN(&proj, NULL,
        "could not allocate memory for perspective projection");

    float fov_rad = glm_rad(fov);
    float aspect = width / height;

    rbtk_projection_specs specs = {
        .type = RBTK_PROJECTION_PERSP,
        .persp = {
            .fov = fov,
            .aspect = aspect,
        },
        .near = near,
        .far = far,
        .width = width,
        .height = height
    };

    proj->specs = specs;
    glm_perspective(fov_rad, aspect, near, far, proj->matrix);
    return proj;
}

void
rbtk_destroy_projection(RBTK_PROJECTION *proj)
{
    free(proj);
}

RBTK_NO_DISCARD const rbtk_projection_specs *
rbtk_get_projection_specs(RBTK_PROJECTION *proj)
{
    return &proj->specs;
}

static bool
load_scene_sprite(RBTK_GRAPHICS *scene)
{
    RBTK_SPRITE *sprite = NULL;
    RBTK_MALLOC_OR_RETURN(&sprite, false,
        "could not allocate memory for scene sprite");

    sprite->scene = scene;
    sprite->width = scene->width;
    sprite->height = scene->height;

    sprite->flipped.vertically = false;
    sprite->flipped.horizontally = false;

    sprite->section.x = 0;
    sprite->section.y = 0;
    sprite->section.width = scene->width;
    sprite->section.height = scene->height;

    sprite->offset.x = 0;
    sprite->offset.y = 0;
    sprite->offset.z = 0;

    sprite->rotation.x = 0;
    sprite->rotation.y = 0;
    sprite->rotation.z = 0;

    sprite->scale.x = 1.0f;
    sprite->scale.y = 1.0f;
    sprite->scale.z = 1.0f;

    sprite->color.red = 1.0f;
    sprite->color.green = 1.0f;
    sprite->color.blue = 1.0f;
    sprite->color.alpha = 1.0f;

    glm_mat4_identity(sprite->model);

    PLAT_RBTK_SPRITE *plat = plat_rbtk_load_sprite(
        sprite->width, sprite->height, 4, NULL);
    if (!plat) {
        free(sprite);
        rbtk_suggest_error(RBTK_ERROR_PLATFORM,
            "could not load scene sprite for current platform");
        return false;
    }

    sprite->plat = plat;
    scene->sprite = sprite;
    return true;
}

RBTK_NO_DISCARD RBTK_GRAPHICS *
rbtk_create_scene(const RBTK_PROJECTION *proj,
    unsigned int width, unsigned int height)
{
    assert(proj);
    REQUIRE_INITIALIZED_OR_RETURN(NULL);

    RBTK_GRAPHICS *scene = malloc(sizeof(*scene));
    if (!scene) {
        rbtk_signal_error(RBTK_ERROR_OUT_OF_MEMORY,
            "could not allocate memory for scene");
        return NULL;
    }

    scene->proj = proj;
    scene->width = width;
    scene->height = height;

    RBTK_CAMERA *camera = malloc(sizeof(*camera));
    if (!camera) {
        free(scene);
        rbtk_signal_error(RBTK_ERROR_OUT_OF_MEMORY,
            "could not allocate memory for camera");
        return NULL;
    }

    float near = proj->specs.near;
    float far = proj->specs.far;

    /*
     * Initialize the camera's Z-axis to the near minus the far to
     * ensure rendering starts at 0.0 on the Z-axis for objects like
     * sprites. Failing to do this results in negative Z-axis values
     * being required for anything to appear.
     */
    camera->pos[0] = +0.0f;        /* X-axis */
    camera->pos[1] = +0.0f;        /* Y-axis */
    camera->pos[2] = (near - far); /* Z-axis */

    scene->camera = camera;

    /* this sets the sprite data field for us also */
    if (!load_scene_sprite(scene)) {
        free(scene);
        free(camera);
        return NULL;
    }

    for (size_t i = 0; i < RBTK_MAX_WINDOW_COUNT; i++) {
        scene->windows[i] = NULL;
    }

    /*
     * We have to do this last as the platform specific data for the
     * graphics scene depends on the other data fields. When finished,
     * the function will set the plat data field for us.
     */
    if (!plat_rbtk_create_scene(scene, width, height)) {
        free(scene);
        free(camera);
        rbtk_suggest_error(RBTK_ERROR_PLATFORM,
            "could not create scene for current platform");
        return NULL;
    }

    return scene;
}

bool
rbtk_destroy_scene(RBTK_GRAPHICS *scene)
{
    assert(scene);

    /*
     * If the number of bindings is greater than zero, it means this scene
     * is still bound to one or more windows. Destruction of the scene now
     * would result in undefined behavior (likely crash) on the next render
     * call for the windows in question.
     */
    size_t num_bindings = 0;
    for (size_t i = 0; i < RBTK_MAX_WINDOW_COUNT; i++) {
        num_bindings += scene->windows[i] ? 1 : 0;
    }
    if (num_bindings > 0) {
        rbtk_signal_error(RBTK_ERROR_ILLEGAL_STATE,
            "scene currently in use by %d window(s)", num_bindings);
        return false;
    }

    if (!plat_rbtk_destroy_scene(scene)) {
        return false;
    }

    free(scene->camera);

    RBTK_SPRITE *sprite = scene->sprite;
    sprite->scene = NULL; /* no longer in use */
    rbtk_unload_sprite(sprite);

    free(scene);
    return true;
}

void
rbtk_clear_scene(RBTK_GRAPHICS *scene)
{
    assert(scene);
    plat_rbtk_clear_scene(scene);
}

RBTK_NO_DISCARD const RBTK_PROJECTION *
rbtk_get_scene_projection(RBTK_GRAPHICS *scene)
{
    assert(scene);
    return scene->proj;
}

RBTK_NO_DISCARD RBTK_CAMERA *
rbtk_get_scene_camera(RBTK_GRAPHICS *scene)
{
    assert(scene);
    return scene->camera;
}

void
rbtk_get_camera_pos(RBTK_CAMERA *camera,
    float *x, float *y, float *z)
{
    assert(camera);
    assert(x || y || z);

    if (x) {
        *x = camera->pos[0];
    }
    if (y) {
        *y = camera->pos[1];
    }
    if (z) {
        *z = camera->pos[2];
    }
}

void
rbtk_set_camera_pos(RBTK_CAMERA *camera,
    float x, float y, float z)
{
    assert(camera);
    camera->pos[0] = x;
    camera->pos[1] = y;
    camera->pos[2] = z;
}

void
rbtk_move_camera(RBTK_CAMERA *camera,
    float x, float y, float z)
{
    assert(camera);
    camera->pos[0] += x;
    camera->pos[1] += y;
    camera->pos[2] += z;
}

void
rbtk_center_camera_greekways(RBTK_GRAPHICS *scene)
{
    assert(scene);

    RBTK_CAMERA *camera = scene->camera;
    const RBTK_PROJECTION *proj = scene->proj;

    if (proj->specs.type == RBTK_PROJECTION_ORTHO) {
        rbtk_set_camera_pos(camera, 0.0f, 0.0f, 0.0f);
    }
    else if (proj->specs.type == RBTK_PROJECTION_PERSP) {
        float width = proj->specs.width;
        float height = proj->specs.height;

        float x_offset = (width / -2.0f);
        float y_offset = (height / -2.0f);
        float z_offset = y_offset;

        rbtk_set_camera_pos(camera, x_offset, y_offset, z_offset);
    }
    else {
        assert(0); /* we forgot one! */
    }
}

RBTK_NO_DISCARD RBTK_SPRITE *
rbtk_get_scene_sprite(RBTK_GRAPHICS *scene)
{
    assert(scene);
    return scene->sprite;
}

void
rbtk_draw_scene(RBTK_GRAPHICS *dest, RBTK_GRAPHICS *src,
    float x, float y, float z)
{
    assert(dest);
    assert(src);
    rbtk_draw_sprite(dest, src->sprite, x, y, z);
}

RBTK_NO_DISCARD RBTK_SPRITE *
rbtk_load_sprite(RBTK_ASSET *asset)
{
    assert(asset);

    RBTK_SPRITE *sprite = NULL;
    RBTK_MALLOC_OR_RETURN(&sprite, NULL,
        "could not allocate memory for sprite");

    RBTK_IN_STREAM *in = rbtk_open_asset_in_stream(asset);
    size_t buffer_size = 0;
    unsigned char *buffer = rbtk_buffer_remaining(in, &buffer_size);
    rbtk_close_in_stream(in);
    if (!buffer) {
        free(sprite);
        return NULL;
    }

    int width, height, channels;
    stbi_uc *img = stbi_load_from_memory(buffer, (int) buffer_size,
        &width, &height, &channels, 0);

    sprite->scene = NULL;
    sprite->width = width;
    sprite->height = height;

    PLAT_RBTK_SPRITE *plat = plat_rbtk_load_sprite(
        (unsigned int) width, (unsigned int) height,
        (unsigned short) channels, img);
    if (!plat) {
        free(sprite);
        stbi_image_free(img);
        rbtk_suggest_error(RBTK_ERROR_PLATFORM,
            "could not load image for current platform");
        return NULL;
    }

    sprite->flipped.vertically = false;
    sprite->flipped.horizontally = false;

    sprite->section.x = 0;
    sprite->section.y = 0;
    sprite->section.width = width;
    sprite->section.height = height;

    sprite->offset.x = 0;
    sprite->offset.y = 0;
    sprite->offset.z = 0;

    sprite->rotation.x = 0;
    sprite->rotation.y = 0;
    sprite->rotation.z = 0;

    sprite->scale.x = 1.0f;
    sprite->scale.y = 1.0f;
    sprite->scale.z = 1.0f;

    sprite->color.red = 1.0f;
    sprite->color.green = 1.0f;
    sprite->color.blue = 1.0f;
    sprite->color.alpha = 1.0f;

    glm_mat4_identity(sprite->model);

    stbi_image_free(img);
    sprite->plat = plat;
    return sprite;
}

bool
rbtk_unload_sprite(RBTK_SPRITE *sprite)
{
    if (!sprite) {
        return true;
    }
    else if (sprite->scene) {
        rbtk_signal_error(RBTK_ERROR_ILLEGAL_ARGUMENT,
            "cannot unload the sprite for a graphics scene");
        return false;
    }

    bool success = plat_rbtk_unload_sprite(sprite);
    free(sprite); /* weo do need this anymore */
    return success;
}

void
rbtk_get_sprite_size(RBTK_SPRITE *sprite,
    unsigned int *width, unsigned int *height)
{
    assert(sprite);
    assert(width || height);
    if (width) {
        *width = sprite->width;
    }
    if (height) {
        *height = sprite->height;
    }
}

void
rbtk_flip_sprite_vertically(RBTK_SPRITE *sprite, bool flip)
{
    assert(sprite);
    sprite->flipped.vertically = flip;
}

void
rbtk_flip_sprite_horizontally(RBTK_SPRITE *sprite, bool flip)
{
    assert(sprite);
    sprite->flipped.horizontally = flip;
}

bool
rbtk_use_sprite_section(RBTK_SPRITE *sprite,
    unsigned int x, unsigned int y,
    unsigned int width, unsigned int height)
{
    assert(sprite);

    if (x + width > sprite->width || y + height > sprite->height) {
        rbtk_signal_error(RBTK_ERROR_OUT_OF_BOUNDS,
            "section out of bounds for %dx%d sprite",
            sprite->width, sprite->height);
        return false;
    }

    sprite->section.x = x;
    sprite->section.y = y;
    sprite->section.width = width;
    sprite->section.height = height;

    plat_rbtk_update_sprite_section(sprite);

    return true;
}

void
rbtk_get_sprite_offset(RBTK_SPRITE *sprite,
    float *x, float *y, float *z)
{
    assert(sprite);
    assert(x || y || z);
    if (x) {
        *x = sprite->offset.x;
    }
    if (y) {
        *y = sprite->offset.y;
    }
    if (z) {
        *z = sprite->offset.z;
    }
}

void
rbtk_set_sprite_offset(RBTK_SPRITE *sprite,
    float x, float y, float z)
{
    assert(sprite);
    sprite->offset.x = x;
    sprite->offset.y = y;
    sprite->offset.z = z;
}

void
rbtk_move_sprite_offset(RBTK_SPRITE *sprite,
    float x, float y, float z)
{
    assert(sprite);
    sprite->offset.x += x;
    sprite->offset.y += y;
    sprite->offset.z += z;
}

void
rbtk_get_sprite_rotation(RBTK_SPRITE *sprite,
    float *x, float *y, float *z)
{
    assert(sprite);
    assert(x || y || z);
    if (x) {
        *x = sprite->rotation.x;
    }
    if (y) {
        *y = sprite->rotation.y;
    }
    if (z) {
        *z = sprite->rotation.z;
    }
}

void
rbtk_rotate_sprite_to(RBTK_SPRITE *sprite,
    float x, float y, float z)
{
    assert(sprite);

    x = (float) fmod(x, 360.0f);
    y = (float) fmod(y, 360.0f);
    z = (float) fmod(z, 360.0f);

    sprite->rotation.x = x;
    sprite->rotation.y = y;
    sprite->rotation.z = z;

}

void
rbtk_rotate_sprite_by(RBTK_SPRITE *sprite,
    float x, float y, float z)
{
    assert(sprite);
    rbtk_rotate_sprite_to(sprite,
        sprite->rotation.x + x,
        sprite->rotation.y + y,
        sprite->rotation.z + z);
}

void
rbtk_scale_sprite(RBTK_SPRITE *sprite,
    float x, float y, float z)
{
    assert(sprite);
    sprite->scale.x = x;
    sprite->scale.y = y;
    sprite->scale.z = z;
}

void
rbtk_scale_sprite_by_size(RBTK_SPRITE *sprite,
    float x, float y, float z) {
    assert(sprite);
    sprite->scale.x = x / sprite->width;
    sprite->scale.y = y / sprite->height;
    sprite->scale.z = z;
}

void
rbtk_set_sprite_color(RBTK_SPRITE *sprite,
    float red, float green, float blue)
{
    assert(sprite);
    sprite->color.red = rbtk_clamp_f32(red, 0.0f, 1.0f);
    sprite->color.green = rbtk_clamp_f32(green, 0.0f, 1.0f);
    sprite->color.blue = rbtk_clamp_f32(blue, 0.0f, 1.0f);
}

void
rbtk_set_sprite_red(RBTK_SPRITE *sprite, float red)
{
    assert(sprite);
    sprite->color.red = rbtk_clamp_f32(red, 0.0f, 1.0f);
}

void
rbtk_set_sprite_green(RBTK_SPRITE *sprite, float green)
{
    assert(sprite);
    sprite->color.green = rbtk_clamp_f32(green, 0.0f, 1.0f);
}

void
rbtk_set_sprite_blue(RBTK_SPRITE *sprite, float blue)
{
    assert(sprite);
    sprite->color.blue = rbtk_clamp_f32(blue, 0.0f, 1.0f);
}

void
rbtk_set_sprite_alpha(RBTK_SPRITE *sprite, float alpha)
{
    assert(sprite);
    sprite->color.alpha = rbtk_clamp_f32(alpha, 0.0f, 1.0f);
}

void
rbtk_draw_sprite(RBTK_GRAPHICS *scene, RBTK_SPRITE *sprite,
    float x, float y, float z)
{
    assert(scene);
    assert(sprite);

    x += sprite->offset.x;
    y += sprite->offset.y;
    z += sprite->offset.z;

    plat_rbtk_draw_sprite(scene, sprite, x, y, z);
}

RBTK_NO_DISCARD RBTK_SPRITE_ANIME *
rbtk_create_sprite_anime(size_t max_frames)
{
    RBTK_SPRITE_ANIME *anime = NULL;
    RBTK_MALLOC_OR_RETURN(&anime, NULL,
        "could not allocate memory for sprite animation");

    RBTK_SPRITE **frames = malloc(max_frames * sizeof(*frames));
    long double *durations = malloc(max_frames * sizeof(*durations));
    if (!frames || !durations) {
        free(anime);
        free(frames);
        free(durations);
        rbtk_signal_error(RBTK_ERROR_OUT_OF_MEMORY,
            "could not allocate memory for frames");
        return NULL;
    }

    anime->max_frames = max_frames;
    anime->num_frames = 0;
    anime->frames = frames;
    anime->durations = durations;
    anime->timer = 0;

    anime->loop = true;
    anime->ping_pong = false;
    anime->backwards = false;
    anime->finished = false;
    anime->current_frame = 0;

    anime->offset.x = 0;
    anime->offset.y = 0;
    anime->offset.z = 0;

    return anime;
}

bool
rbtk_destroy_sprite_anime(RBTK_SPRITE_ANIME *anime, bool unload_sprites)
{
    if (unload_sprites) {
        for (size_t i = 0; i < anime->num_frames; i++) {
            RBTK_SPRITE *sprite = anime->frames[i];
            if (!rbtk_unload_sprite(sprite)) {
                return false;
            }
        }
    }

    free(anime->frames);
    free(anime->durations);
    free(anime);

    return true;
}

bool
rbtk_add_sprite_to_anime(RBTK_SPRITE_ANIME *anime, RBTK_SPRITE *sprite,
    long double duration, rbtk_time_unit unit)
{
    assert(anime);
    assert(sprite);

    if (duration <= 0) {
        rbtk_signal_error(RBTK_ERROR_ILLEGAL_ARGUMENT,
            "frame duration must be positive");
        return false;
    }

    size_t slot = anime->num_frames;
    if (slot >= anime->max_frames) {
        rbtk_signal_error(RBTK_ERROR_OUT_OF_MEMORY,
            "max frame count %d reached", anime->max_frames);
        return false;
    }

    anime->frames[slot] = sprite;
    anime->durations[slot] = rbtk_convert_time(unit, RBTK_MILLIS, duration);
    anime->num_frames += 1;

    return true;
}

RBTK_NO_DISCARD RBTK_SPRITE *
rbtk_get_sprite_in_anime(RBTK_SPRITE_ANIME *anime, size_t index)
{
    assert(anime);
    if (index >= anime->num_frames) {
        rbtk_signal_error(RBTK_ERROR_OUT_OF_BOUNDS,
            "index %d exceeds frame count %d", index, anime->num_frames);
        return NULL;
    }
    return anime->frames[index];
}

bool
rbtk_set_sprite_in_anime(RBTK_SPRITE_ANIME *anime, size_t index,
    RBTK_SPRITE *sprite)
{
    assert(anime);
    if (index >= anime->num_frames) {
        rbtk_signal_error(RBTK_ERROR_OUT_OF_BOUNDS,
            "index %d exceeds frame count %d", index, anime->num_frames);
        return false;
    }
    anime->frames[index] = sprite;
    return true;
}

RBTK_NO_DISCARD long double
rbtk_get_sprite_duration_in_anime(RBTK_SPRITE_ANIME *anime, size_t index,
    rbtk_time_unit unit)
{
    assert(anime);
    if (index >= anime->num_frames) {
        rbtk_signal_error(RBTK_ERROR_OUT_OF_BOUNDS,
            "index %d exceeds frame count %d", index, anime->num_frames);
        return -1;
    }
    long double duration = anime->durations[index];
    return rbtk_convert_time(RBTK_MILLIS, unit, duration);
}

bool
rbtk_set_sprite_duration_in_anime(RBTK_SPRITE_ANIME *anime, size_t index,
    long double duration, rbtk_time_unit unit)
{
    assert(anime);

    if (index >= anime->num_frames) {
        rbtk_signal_error(RBTK_ERROR_OUT_OF_BOUNDS,
            "index %d exceeds frame count %d", index, anime->num_frames);
        return false;
    }

    if (duration <= 0) {
        rbtk_signal_error(RBTK_ERROR_ILLEGAL_ARGUMENT,
            "frame duration must be positive");
        return false;
    }

    anime->durations[index] = rbtk_convert_time(unit, RBTK_MILLIS, duration);
    return true;
}

RBTK_NO_DISCARD bool
rbtk_sprite_anime_is_finished(RBTK_SPRITE_ANIME *anime) {
    assert(anime);
    return anime->finished;
}

RBTK_NO_DISCARD size_t
rbtk_get_current_sprite_anime_index(RBTK_SPRITE_ANIME *anime)
{
    assert(anime);
    return anime->current_frame;
}

bool
rbtk_set_current_sprite_anime_index(RBTK_SPRITE_ANIME *anime, size_t index)
{
    assert(anime);

    if (index >= anime->num_frames) {
        rbtk_signal_error(RBTK_ERROR_OUT_OF_BOUNDS,
            "index %d out of bounds for animation with %d frames",
            index, anime->num_frames);
        return false;
    }

    anime->timer = 0;
    anime->finished = false;
    anime->current_frame = index;
    return true;
}

void
rbtk_restart_sprite_anime(RBTK_SPRITE_ANIME *anime)
{
    assert(anime);
    if (anime->num_frames > 0) {
        anime->timer = 0;
        anime->finished = false;
        if (anime->backwards){
            anime->current_frame = anime->num_frames - 1;
        } else {
            anime->current_frame = 0;
        }
    }
}

RBTK_NO_DISCARD RBTK_SPRITE *
rbtk_get_current_sprite_anime_frame(RBTK_SPRITE_ANIME *anime)
{
    assert(anime);
    return anime->frames[anime->current_frame];
}

bool
rbtk_sprite_anime_is_looping(RBTK_SPRITE_ANIME *anime, bool *ping_pong)
{
    assert(anime);
    if (ping_pong) {
        *ping_pong = anime->ping_pong;
    }
    return anime->loop;
}

void
rbtk_loop_sprite_anime(RBTK_SPRITE_ANIME *anime, bool loop, bool ping_pong)
{
    assert(anime);
    if (loop) {
        anime->loop = true;
        anime->ping_pong = ping_pong;
    } else {
        anime->loop = false;
        anime->ping_pong = false;
    }
}

bool
rbtk_sprite_anime_is_playing_backwards(RBTK_SPRITE_ANIME *anime)
{
    assert(anime);
    return anime->backwards;
}

void
rbtk_play_sprite_anime_backwards(RBTK_SPRITE_ANIME *anime, bool backwards)
{
    assert(anime);
    anime->backwards = backwards;
}

void
rbtk_get_sprite_anime_offset(RBTK_SPRITE_ANIME *anime,
    float *x, float *y, float *z)
{
    assert(anime);
    assert(x || y || z);
    if (x) {
        *x = anime->offset.x;
    }
    if (y) {
        *y = anime->offset.y;
    }
    if (z) {
        *z = anime->offset.z;
    }
}

void
rbtk_set_sprite_anime_offset(RBTK_SPRITE_ANIME *anime,
    float x, float y, float z)
{
    assert(anime);
    anime->offset.x = x;
    anime->offset.y = y;
    anime->offset.z = z;
}

void
rbtk_move_sprite_anime_offset(RBTK_SPRITE_ANIME *anime,
    float x, float y, float z)
{
    assert(anime);
    anime->offset.x += x;
    anime->offset.y += y;
    anime->offset.z += z;
}

void
rbtk_update_sprite_anime(RBTK_SPRITE_ANIME *anime, long double delta,
    rbtk_time_unit unit)
{
    assert(anime);
    assert(delta >= 0);

    long double delta_ms = rbtk_convert_time(unit, RBTK_MILLIS, delta);

    size_t num_frames = anime->num_frames;
    long double timer = anime->timer += delta_ms;
    bool backwards = anime->backwards;
    bool finished = anime->finished;
    size_t current_frame = anime->current_frame;

    while (timer >= anime->durations[current_frame]) {
        timer -= anime->durations[current_frame];
        current_frame += anime->backwards ? -1 : 1;

        /*
         * If the current frame is SIZE_MAX, it means it has gone into
         * the negatives (which is why it has wrapped around to the max
         * value). Now, this should mean that it was playing backwards.
         * Depending on the settings for this animation, we must take a
         * specific course of action.
         */
        if (current_frame == SIZE_MAX) {
            assert(backwards);

            /*
             * First, add one to ensure the current frame remains valid.
             * Next, if looping is not enabled just break out of the loop
             * and set the timer back to zero so this process can repeat
             * naturally (therefore handling changes to the settings).
             */
            current_frame += 1;
            if (!anime->loop) {
                timer = 0;       /* allow process to repeat */
                finished = true; /* animation is done       */
                break;           /* do no more processing   */
            }

            /*
             * If ping ponging is enabled, go forward one more frame since
             * we just finished playing the first frame and set backwards
             * to false so it starts playing forwards on the next update.
             * If ping ponging is disabled, then set the current frame to
             * the last (and also set backwards to true for redundancy).
             */
            finished = false;
            if (anime->ping_pong) {
                current_frame += 1;
                backwards = false;
            } else {
                current_frame = num_frames - 1;
                backwards = true;
            }
        }

        /*
         * If the current frame has gone past the frame count, it means
         * the animation has reached the end. Depending on the settings
         * for this animation, we must take a specific course of action.
         */
        if (current_frame >= num_frames) {
            assert(!backwards);

            /*
             * First, subtract one to ensure the current frame remains
             * valid. Next, if looping is not enabled just break out of
             * the loop and set the timer back to zero so this process
             * can repeat naturally (therefore handling changes to the
             * animation's settings).
             */
            current_frame -= 1;
            if (!anime->loop) {
                timer = 0;       /* allow process to repeat */
                finished = true; /* animation is done       */
                break;           /* do no more processing   */
            }

            /*
             * If ping ponging is enabled, go back one more frame since
             * we just finished playing the last frame and set backwards
             * to true so it keeps playing backwards on the next update.
             * If ping ponging is disabled, then set the current frame to
             * the first (and also set backwards to false for redundancy).
             */
            finished = false;
            if (anime->ping_pong) {
                current_frame -= 1;
                backwards = true;
            } else {
                current_frame = 0;
                backwards = false;
            }
        }
    }

    anime->timer = timer;
    anime->backwards = backwards;
    anime->finished = finished;
    anime->current_frame = current_frame;
}

void
rbtk_draw_sprite_anime(RBTK_GRAPHICS *scene, RBTK_SPRITE_ANIME *anime,
    float x, float y, float z)
{
    assert(scene);
    assert(anime);

    x += anime->offset.x;
    y += anime->offset.y;
    z += anime->offset.z;

    RBTK_SPRITE *sprite = anime->frames[anime->current_frame];
    rbtk_draw_sprite(scene, sprite, x, y, z);
}
