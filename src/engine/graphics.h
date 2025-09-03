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
#ifndef RBTK_ENGINE_GRAPHICS_H_
#define RBTK_ENGINE_GRAPHICS_H_

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/*!
 * @file
 * @brief The public API for the game engine's graphics module.
 */

#include <stdbool.h>

#include "../runtime/common.h"
#include "../runtime/asset.h"
#include "../runtime/time.h"

/*!
 * @defgroup engine_grpahics Graphics System
 * @brief The game engine's graphics module.
 *
 * @{
 */

/*!
 * @brief The most monitors which can be recognized at one time.
 *
 * Whenever this number would be exceeded, the graphics module simply acts
 * as though the extra monitors do not exist. The primary monitor is always
 * the first to be added, so it will never be excluded.
 *
 * @warning If you send me a pull request to support more monitors, I will
 * come to your house and smash `N - 16` of your monitors with a hammer.
 */
#define RBTK_MAX_MONITOR_COUNT 16

/*!
 * @brief The most windows which can exist at one time.
 *
 * Whenever this number would be exceeded, the graphics module will refuse
 * to create the window and signals an error. A different system for storing
 * the current windows will be used in the future if this becomes an issue.
 *
 * @see rbtk_create_window(unsigned int, unsigned int, const char *)
 */
#define RBTK_MAX_WINDOW_COUNT  32

/*!
 * @brief Represents a monitor.
 *
 * Monitors (usually) represent physical screens attached to a device.
 * These are primarily the containers for windows.
 *
 * @note Depending on the platform, a monitor may contain zero windows,
 * one window exactly, or an arbitrary number of windows.
 *
 * @see RBTK_WINDOW
 */
RBTK_FORWARD_DECLARATION
typedef struct RBTK_MONITOR RBTK_MONITOR;

/*!
 * @brief An unmodifable array of monitors.
 *
 * @see rbtk_get_monitors(size_t)
 */
typedef RBTK_MONITOR *const * RBTK_MONITORS;

/*!
 * @brief Represents a window.
 *
 * Windows are used to contain the content displayed by an application. They
 * can also perform a range of different operations. However, which features
 * is depdendent on the current platform.
 *
 * @see rbtk_create_window(unsigned int, unsigned int, const char *)
 * @see RBTK_MONITOR
 */
RBTK_FORWARD_DECLARATION
typedef struct RBTK_WINDOW RBTK_WINDOW;

/*!
 * @brief An unmodifable array of windows.
 *
 * @see rbtk_get_windows(size_t)
 */
typedef RBTK_WINDOW *const * RBTK_WINDOWS;

/*!
 * @brief Possible display modes for a window.
 */
typedef enum rbtk_display_mode {
    RBTK_WINDOWED,
    RBTK_FULLSCREEN,
    RBTK_BORDERLESS_FULLSCREEN,
} rbtk_display_mode;

/*!
 * @brief Describes window capabilities.
 *
 * The primary purpose of a window is to contain content displayed by an
 * application. However, depending on the platform, they can also perform
 * a range of different operations.
 *
 * @note The capabilities only describe if a window on the current platform
 * can *theoretically* perform an action. Although a window could be resized,
 * an application may make said window non-resizeable.
 *
 * @see rbtk_get_window_caps(const RBTK_WINDOW *)
 */
typedef struct rbtk_window_caps {
    bool can_minimize;   /*!< The window can be minimized.               */
    bool can_close;      /*!< The window can be closed.                  */
    bool can_move;       /*!< The window can be moved across the screen. */
    bool resizeable;     /*!< The window's dimensions can be changed.    */
    bool has_title;      /*!< The window has a visible title.            */
    bool has_icon;       /*!< The window has a visible icon.             */
} rbtk_window_caps;

/*!
 * @brief Describes the different types of matrices.
 *
 * The projection matrix determines how the camera (or the user) will see the
 * world around it. The two projection matrices available are the orthographic
 * and perspective matrices. In an orthographic matrix, all objects appear the
 * same distance away from the camera. In a perpsective matrix, objects appear
 * as though they have real distance from each other.
 *
 * @note Orthogrpahic matrices are generally useful for rendering 2D objects,
 * while perspective matrices are generaly useful for rendering 3D objects.
 * However, they are not "one size fits all". There might be times you will
 * want to draw 2D objects in a 3D environment, or vice-versa.
 *
 * @see rbtk_projection_specs
 * @see rbtk_create_ortho_projection(float, float, float, float, float, float)
 * @see rbtk_create_persp_projection(float, float, float, float, float)
 */
typedef enum rbtk_projection_type {
    RBTK_PROJECTION_ORTHO, /*!< Orthographic projection matrix. */
    RBTK_PROJECTION_PERSP, /*!< Perspective projection matrix. */
} rbtk_projection_type;

/*!
 * @brief Represents a projection matrix.
 *
 * Projection matrices are used to define the base coordinates to use when
 * drawing. The default projection matrix starts with a center of `(0, 0)`.
 * `+1` is the top as well as the right, and `-1` is the bottom as well as
 * the left. This can be changed by creating your own and applying it.
 *
 * @see rbtk_get_scene_projection(const RBTK_GRAPHICS *)
 * @see rbtk_create_ortho_matrix(float, float, float, float, float, float)
 * @see rbtk_create_persp_projection(float, float, float, float, float)
 * @see RBTK_CAMERA
 */
RBTK_FORWARD_DECLARATION
typedef struct RBTK_PROJECTION RBTK_PROJECTION;

/*!
 * @brief The specifications for a projection matrix.
 *
 * @see rbtk_get_projection_specs(const RBTK_PROJECTION *)
 */
typedef struct rbtk_projection_specs {
    rbtk_projection_type type;
    union {
	struct {
	    float left;   /*!< The leftmost coordinate.       */
	    float right;  /*!< The rightmost coordinate.      */
	    float top;    /*!< The topmost coordinate.        */
	    float bottom; /*!< The bottommost coordinate.     */
        } ortho;
        struct {
            float fov;    /*!< The field of view in degrees.  */
	    float aspect; /*< The aspect ratio.               */
        } persp;
    };
    float near;   /*!< The nearest Z-value before clipping.   */
    float far;    /*!< The farthest Z-value before clipping.  */
    float width;  /*!< The width of the viewport, in pixels.  */
    float height; /*!< The height of the viewport, in pixels. */
} rbtk_projection_specs;

/*!
 * @brief Represents a camera.
 *
 * Cameras are used to move around the physical space of a world, without
 * moving the world itself. They are often used in-tandem with projection
 * matrices, which define how to interpret coordinates.
 *
 * @see rbtk_get_scene_camera(const RBTK_GRAPHICS *)
 * @see RBTK_PROJECTION
 */
RBTK_FORWARD_DECLARATION
typedef struct RBTK_CAMERA RBTK_CAMERA;

/*!
 * @brief Represents a graphics context.
 *
 * A graphics context is used to hold the state of the graphics engine the
 * program uses for rendering. At the moment, the engine uses only a single
 * graphics context for all rendering. However, this may change later on.
 *
 * @see rbtk_create_scene(const RBTK_PROJECTION *)
 * @see RBTK_CAMERA
 */
RBTK_FORWARD_DECLARATION
typedef struct RBTK_GRAPHICS RBTK_GRAPHICS;

/*!
 * @brief Represents a sprite.
 *
 * Sprites a 2D images with a width and height. They are drawn by themselves
 * and are not to be applied to a 3D model. While not required, they are also
 * often part of a spritesheet; which contains many sprites.
 *
 * @see rbtk_load_sprite(const RBTK_ASSET *)
 * @see RBTK_SPRITESHEET
 * @see RBTK_SPRITE_ANIME
 */
RBTK_FORWARD_DECLARATION
typedef struct RBTK_SPRITE RBTK_SPRITE;

/*!
 * @brief Represents a spritesheet.
 *
 * Spritesheets are containers for multiple sprites. These come in handy
 * when characters are made up of many sprites or have animations. Using
 * spritesheets also allows the graphics module to make optimizations in
 * regards to speed and memory usage.
 *
 * @note This feature is a work-in-progress.
 *
 * @see RBTK_SPRITE
 */
RBTK_FORWARD_DECLARATION
typedef struct RBTK_SPRITESHEET RBTK_SPRITESHEET;

/*!
 * @brief Represents an animation of sprites.
 *
 * These are made up of multiple sprites. Each sprite in an animation has
 * its own duration. Once the specified duration for a sprite has passed
 * between updates, the animation will go to the next sprite. This process
 * goes on forever unless otherwise specified not to loop. The same sprite
 * can be used for multiple frames.
 *
 * @see rbtk_create_sprite_anime(size_t)
 * @see RBTK_SPRITE
 */
RBTK_FORWARD_DECLARATION
typedef struct RBTK_SPRITE_ANIME RBTK_SPRITE_ANIME;

/*!
 * @brief Returns all current monitors.
 *
 * @param[out] count The number of monitors.
 * @return A pointer to the current monitors, `NULL` on error.
 *
 * @pointer_lifetime The returned array is valid for the duration of the
 * program. The pointers contained in the array are valid for the duration
 * as specified by #rbtk_get_monitor(size_t).
 *
 * @debugging This function asserts that `count` is not `NULL`.
 * @errors
 * @signal{#RBTK_ERROR_ILLEGAL_STATE, If the graphics module is not
 *                                    initialized.}
 * @enderrors
 */
RBTK_NO_DISCARD RBTK_MONITORS
rbtk_get_monitors(size_t *count);

/*!
 * @brief Returns a monitor by its index.
 *
 * @param[in] index The index of the monitor, `0` for the primary monitor.
 * @return The requested monitor, `NULL` on error.
 *
 * @pointer_lifetime The returned mointor is valid until it disconnects or
 * the graphics module is terminated.
 *
 * @errors
 * @signal{#RBTK_ERROR_ILLEGAL_STATE, If the graphics module is not
 *                                    initialized.}
 * @signal{#RBTK_ERROR_OUT_OF_BOUNDS, If `index` is greater than or equal
 *                                    to the number of existing monitor.}
 * @enderrors
 */
RBTK_NO_DISCARD RBTK_MONITOR *
rbtk_get_monitor(size_t index);

/*!
 * @brief Returns the primary monitor.
 *
 * The primary monitor is chosen by the graphics module at initialization.
 * It is usually the monitor which houses the primary window, but this is
 * not a requirement.
 *
 * @return The primary monitor, `NULL` on error.
 *
 * @pointer_lifetime The returned mointor is valid until it disconnects or
 * the graphics module is terminated.
 *
 * @errors
 * @signal{#RBTK_ERROR_ILLEGAL_STATE, If the graphics module is not
 *                                    initialized.}
 * @enderrors
 */
#define rbtk_get_primary_monitor() rbtk_get_monitor(0)

/*!
 * @brief Returns all current windows.
 *
 * @param[out] count The number of windows.
 * @return A pointer to the current windows, `NULL` on error.
 *
 * @pointer_lifetime The returned array is valid for the duration of the
 * program. The pointers contained in the array are valid for the duration
 * as specified by #rbtk_get_window(size_t).
 *
 * @debugging This function asserts that `count` is not `NULL`.
 * @errors
 * @signal{#RBTK_ERROR_ILLEGAL_STATE, If the graphics module is not
 *                                    initialized.}
 * @enderrors
 */
RBTK_NO_DISCARD RBTK_WINDOWS
rbtk_get_windows(size_t *count);

/*!
 * @brief Returns a window by its index.
 *
 * @param[in] index The index of the window, `0` for the primary window.
 * @return The requested window, `NULL` on error.
 *
 * @pointer_lifetime The returned window is valid until it is destroyed or
 * the graphics module is terminated.
 *
 * @errors
 * @signal{#RBTK_ERROR_ILLEGAL_STATE, If the graphics module is not
 *                                    initialized.}
 * @signal{#RBTK_ERROR_OUT_OF_BOUNDS, If `index` is greater than or equal
 *                                    to the number of existing windows.}
 * @enderrors
 */
RBTK_NO_DISCARD RBTK_WINDOW *
rbtk_get_window(size_t index);

/*!
 * @brief Returns the primary window.
 *
 * The primary window is always the first window created by the graphics
 * module during initialization.
 *
 * @return The primary window, `NULL` on error.
 *
 * @pointer_lifetime The returned window is valid until it is destroyed or
 * the graphics module is terminated.
 *
 * @errors
 * @signal{#RBTK_ERROR_ILLEGAL_STATE, If the graphics module is not
 *                                    initialized.}
 * @enderrors
 */
#define rbtk_get_primary_window() rbtk_get_window(0)

/*!
 * @brief Creates a window.
 *
 * @attention Depending on the platform, it may not be possible to create
 * another window. If this is the case, `NULL` is returned and an error is
 * signalled.
 *
 * @param[in] width  The window width.
 * @param[in] height The window height.
 * @param[in] title  The window title, may be `NULL`.
 * @return The created window, `NULL` on error.
 *
 * @pointer_lifetime The returned window is valid until it is destroyed via
 * #rbtk_destroy_window(RBTK_WINDOW *) or the graphics module is terminated.
 *
 * @errors
 * @signal{#RBTK_ERROR_ILLEGAL_STATE, If the graphics module is not
 *                                    initialized.}
 * @signal{#RBTK_ERROR_UNSUPPORTED,   If the graphics module cannot create
 *                                    another window on this platform.}
 * @signal{#RBTK_ERROR_PLATFORM,      If the window could not be created
 *                                    and no other error was signalled.}
 * @enderrors
 */
RBTK_NO_DISCARD RBTK_WINDOW *
rbtk_create_window(unsigned int width, unsigned int height,
	const char *title);

/*!
 * @brief Destroys a window.
 *
 * @attention Depending on the platform and window, it may not be possible
 * to destroy the returned window. If this is the case, `false` is returend
 * and an error is signalled.
 *
 * @param[in] window The window to destroy, may be `NULL`.
 * @return `true` on success, `false` on failure.
 *
 * @errors
 * @signal{#RBTK_ERROR_ILLEGAL_ARGUMENT, If the graphics module cannot destroy
 *                                       the given window. This is usually just
 *                                       signalled for the primary window.}
 * @enderrors
 */
bool
rbtk_destroy_window(RBTK_WINDOW *window);

/*!
 * @brief Returns the capabilities of a window.
 *
 * @pointer_lifetime The returned capabilities are valid until the
 * window it belongs to is destroyed.
 *
 * @debugging This funcion asserts that `window` is not `NULL`.
 */
RBTK_NO_DISCARD const rbtk_window_caps *
rbtk_get_window_caps(const RBTK_WINDOW *window);

/*!
 * @brief Returns if a window is visible.
 *
 * @param[in] window The window to query.
 * @return `true` if the window is visible, `false` otherwise.
 *
 * @debugging This function asserts that `window` is not `NULL`.
 */
RBTK_NO_DISCARD bool
rbtk_window_is_visible(RBTK_WINDOW *window);

/*!
 * @brief Shows a window.
 *
 * @param[in] window The window to display.
 * @return `true` on success, `false` on failure.
 *
 * @debugging This function asserts that `window` is not `NULL`.
 * @errors
 * @signal{#RBTK_ERROR_PLATFORM, If the current platform cannot
 *                               hide the window.}
 * @enderrors
 */
bool
rbtk_show_window(RBTK_WINDOW *window);

/*!
 * @brief Hides a window.
 *
 * @param[in] window The window to hide.
 * @return `true` on success, `false` on failure.
 * @errors
 * @signal{#RBTK_ERROR_PLATFORM, If the current platform cannot
 *                               hide the window.}
 * @enderrors
 */
bool
rbtk_hide_window(RBTK_WINDOW *window);

/*!
 * @brief Sets a window's display mode.
 *
 * @param[in] window The window to update.
 * @param[in] mode   The display mode to use.
 * @return `true` on success, `false` on failure.
 *
 * @debugging This function asserts that `window` is not `NULL`.
 * @errors
 * @signal{#RBTK_ERROR_PLATFORM, If the current platform cannot
 *                               set the window to the requested
 *                               display mode.}
 * @enderrors
 */
bool
rbtk_set_display_mode(RBTK_WINDOW *window, rbtk_display_mode mode);

/*!
 * @brief Returns the display mode of a window.
 *
 * @param[in] window The window to query.
 *
 * @debugging This function asserts that `window` is not `NULL`.
 */
RBTK_NO_DISCARD rbtk_display_mode
rbtk_get_display_mode(RBTK_WINDOW *window);

/*!
 * @brief Returns if a window should close.
 *
 * When a window indicates that it should close, it is a request and not a
 * command. The program can (but should not) choose to ignore it completely.
 * Some programs use this indicator to first confirm with the user if they
 * really wish to close the application first.
 *
 * @param[in] window The window to query.
 * @return `true` if the window should close, `false` otherwise.
 *
 * @debugging This function asserts that `window` is not `NULL`.
 *
 * @see rbtk_any_windows_should_close(void)
 */
RBTK_NO_DISCARD bool
rbtk_window_should_close(RBTK_WINDOW *window);

/*!
 * @brief Returns if any of the current windows should close.
 *
 * @return `true` if any windows should close, `false` otherwise.
 *
 * @see rbtk_window_should_close(RBTK_WINDOW *)
 */
RBTK_NO_DISCARD bool
rbtk_any_windows_should_close(void);

/*!
 * @brief Sets if a window should close or not.
 *
 * @param[in] window The window to update.
 * @param[in] close  `true` if the window should close, `false` otherwise.
 *
 * @debugging This function asserts that `window` is not `NULL`.
 */
void
rbtk_set_window_should_close(RBTK_WINDOW *window, bool close);

/*!
 * @brief Sets the icon of a window.
 *
 * @param[in] window The window to update.
 * @param[in] asset  The icon image asset, this may be `NULL` to remove
 *                   the current window icon.
 * @return `true` on success, `false` on failure.
 *
 * @debugging This function asserts that `window` is not `NULL`.
 * @errors
 * @signal{#RBTK_ERROR_UNSUPPORTED, If the graphics module cannot set the
 *                                  window icon on this platform.}
 * @enderrors
 */
RBTK_PLATFORM bool
rbtk_set_window_icon(RBTK_WINDOW *window, RBTK_ASSET *asset);

/*!
 * @brief Returns the title of a window.
 *
 * @param[in] window the window to query.
 * @return The window title, may be `NULL` for none.
 *
 * @pointer_lifetime The returned pointer is valid until the window
 * is destroyed. The memory for this title is managed by the window,
 * it is an unchecked runtime error to free it.
 *
 * @debugging This function asserts that `window` is not `NULL`.
 */
RBTK_NO_DISCARD const char *
rbtk_get_window_title(const RBTK_WINDOW *window);

/*!
 * @brief Sets the title of a window.
 *
 * @note The given title string is copied by this function and manged
 * by the module afterwards. This means the caller must free the memory
 * associated with the title pointer (assuming it lives on the heap).
 *
 * @param[in] window The window to update.
 * @param[in] title  The new window title, may be `NULL`.
 * @return `true` on success, `false` on failure.
 *
 * @debugging This function asserts that `window` is not `NULL`.
 * @errors
 * @signal{#RBTK_ERROR_UNSUPPORTED, If the graphics module cannot set the
 *                                  window title on this platform.}
 * @enderrors
 */
bool
rbtk_set_window_title(RBTK_WINDOW *window, const char *title);

/*!
 * @brief Returns the size of a window in pixels.
 *
 * @param[in]  window The window to query.
 * @param[out] width  The window width, may be `NULL`.
 * @param[out] height The window height, may be `NULL`.
 * @return `true` on success, `false` on failure.
 *
 * @debugging This function asserts that `window` is not `NULL`
 * and that at least one of the dimensions are not `NULL`.
 */
bool
rbtk_get_window_size(const RBTK_WINDOW *window,
    unsigned int *width, unsigned int *height);

/*!
 * @brief Resizes a window.
 *
 * @param[in] window The window to resize.
 * @param[in] width  The new width, `0` to leave as-is.
 * @param[in] height The new height, `0` to leave as-is.
 * @return `true` on success, `false` on failure.
 *
 * @debugging This function asserts that `window` is not `NULL`.
 * @errors
 * @signal{#RBTK_ERROR_UNSUPPORTED, If the graphics module cannot reize
 *                                  the window on this platform.}
 * @enderrors
 */
bool
rbtk_set_window_size(RBTK_WINDOW *window,
    unsigned int width, unsigned int height);

/*!
 * @brief Returns the graphics scene for a window.
 *
 * @param[in] window The window to query.
 * @return The graphics scene displayed by the window, may be `NULL`.
 *
 * @pointer_lifetime The returned graphics scene is valid until it is
 * destroyed or the graphics module is terminated.
 *
 * @debugging This function asserts that `window` is not `NULL`.
 */
RBTK_GRAPHICS *
rbtk_get_window_scene(const RBTK_WINDOW *window);

/*!
 * @brief Binds a graphics scene to a window.
 *
 * Once a scene is bound to the window, calling
 * #rbtk_render_window_scene(const RBTK_WINDOW *) will result in the contents
 * of that scene being shown in that window. If the scene is smaller than the
 * window, it will be resized according to the window's settings.
 *
 * @note A graphics scene can be bound to multiple windows at a time.
 *
 * @param[in] window The window to bind  the scene to.
 * @param[in] scene  The scene being bound, `NULL` to unbind.
 *
 * @debugging This function asserts that `window` is not `NULL`.
 */
void
rbtk_bind_scene_to_window(RBTK_WINDOW *window, RBTK_GRAPHICS *scene);

/*!
 * @brief Draws the current contents of the scene to the window.
 *
 * @note If no graphics scene is currently bound to the window, then nothing
 * will be displayed and previous contents (if any) will be cleared.
 *
 * @param[in] window The window whose contents to update.
 * @return `true` if anything was drawn, `false` otherwise.
 *
 * @debugging This function asserts that `window` is not `NULL`.
 * @errors
 * @signal{#RBTK_ERROR_PLATFORM,      If an error occured while rendering
 *                                    to the window and no other error was
 *                                    signalled.}
 * @enderrors
 */
bool
rbtk_render_window_scene(const RBTK_WINDOW *window);

/*!
 * @brief Clears the scene currently bound to the window.
 *
 * @note If no scene is bound to the window, this function is a no-op.
 *
 * @param[in] window The window whose scene to clear.
 *
 * @debugging This function asserts that `window` is not `NULL`.
 *
 * @see rbtk_clear_scene(RBTK_GRAPHICS *)
 */
void
rbtk_clear_window_scene(const RBTK_WINDOW *window);

/*!
 * @brief Creates an orthographic projection matrix.
 *
 * @param[in] left   The X-axis coordinate's leftmost value.
 * @param[in] right  The X-axis coordinate's rightmost value.
 * @param[in] top    The Y-axis coordinate's topmost value.
 * @param[in] bottom The Y-axis coordinate's bottommost value.
 * @param[in] near   The Z-axis' nearest value before clipping occurs.
 * @param[in] far    The Z-axis' farthest value before clipping occurs.
 * @return The created projection.
 *
 * @pointer_lifetime The returned projection is valid until it is destroyed
 * via #rbtk_destroy_projection(RBTK_PROJECTION *) or the graphics module is
 * terminated.
 *
 * @errors
 * @signal{#RBTK_ERROR_ILLEGAL_STATE, If the graphics module is not
 *                                    initialized.}
 * @enderrors
 */
RBTK_NO_DISCARD RBTK_PROJECTION *
rbtk_create_ortho_projection(float left, float right, float top, float bottom,
    float near, float far);

/*!
 * @brief Creates a greek orthographic matrix.
 *
 * For greek-based orthographic projection matrices:
 *   - The leftmost coordinate is `0.0`.
 *   - The rightmost coordinate is `(_width)`.
 *   - The topmost coordinate is `(_height)`.
 *   - The bottommost coordinate is `0.0`.
 *   - The Z-axis near value is `0.1f`.
 *   - The Z-axis far value is `(_far)`.
 *
 * The term "greek" is used as these matrices are based on the fact that
 * Greek (among other languages in Europe) is written from right to left.
 * This matrix does the same as it starts from the top left and draws to
 * the bottom right.
 *
 * @attention For objects not to be clipped, their Z-axis *must* be greater
 * than the Z-axis near value and must be less than or equal to the Z-axis
 * far value. The camera's position on the Z-axis must also be greater than
 * the object's position on the Z-axis.
 *
 * @param[in] _width  The width of the viewport.
 * @param[in] _height The height of the viewport.
 * @param[in] _far    The Z-axis far value.
 * @return The created projection matrix.
 *
 * @pointer_lifetime The returned matrix is valid until it is
 * destroyed via #rbtk_destroy_projection(RBTK_PROJECTION *) or
 * the graphics module is terminated.
 *
 * @errors
 * @signal{#RBTK_ERROR_ILLEGAL_STATE, If the graphics module is not
 *                                    initialized.}
 * @enderrors
 */
#define rbtk_create_greek_matrix(_width, _height, _far) \
    rbtk_create_ortho_projection(0.0f,      (_width),   \
                                 (_height), 0.0f,       \
                                 0.1f,      (_far))

/*!
 * @brief Creates a perspective projection matrix.
 *
 * @param[in] fov    The field of view, in degrees.
 * @param[in] width  The projection width in pixels.
 * @param[in] height The projection height in pixels.
 * @param[in] near   The Z-axis' nearest value before clipping occurs.
 * @param[in] far    The Z-axis' farthest value before clipping occurs.
 * @return The created projection.
 *
 * @pointer_lifetime The returned projection is valid until it is destroyed
 * via #rbtk_destroy_projection(RBTK_PROJECTION *) or the graphics module is
 * terminated.
 *
 * @errors
 * @signal{#RBTK_ERROR_ILLEGAL_STATE, If the graphics module is not
 *                                    initialized.}
 * @enderrors
 */
RBTK_NO_DISCARD RBTK_PROJECTION *
rbtk_create_persp_projection(float fov, float width, float height,
    float near, float far);

/*!
 * @brief Destroys a projection matrix.
 *
 * @param[in] proj The matrix to destroy.
 *
 * @errors
 * @signal{#RBTK_ERROR_ILLEGAL_STATE, If the matrix is currently
 *                                    in use by a graphics scene.}
 * @enderrors
 */
void
rbtk_destroy_projection(RBTK_PROJECTION *proj);

/*!
 * @brief Returns the specifications for a projection matrix.
 *
 * @param[in] proj The matrix to query.
 * @return The specifications for the given matrix.
 *
 * @pointer_lifetime The returned specifications valid until the
 * projection matrix it belongs to is destroyed.
 *
 * @debugging This function asserts that `proj` is not `NULL`.
 */
RBTK_NO_DISCARD const rbtk_projection_specs *
rbtk_get_projection_specs(RBTK_PROJECTION *proj);

/*!
 * @brief Creates a graphics scene.
 *
 * A graphics scene is used to contain the result of rendering operations.
 * Each window has an associated graphics scene, which is the final place
 * for rendering operations before a flush occurs. More graphic scenes can
 * be created for pre-rendering.
 *
 * @note The `width` and `height` parameters do *not* determine the right
 * and top of the scene's viewport. They refer to the width and height of
 * the texture which contains it. You must keep this in mind when drawing
 * one scene to another!
 *
 * @warning The projection matrix must not be destroyed while in use by a
 * graphics scene. It is an unchecked runtime error to do so.
 *
 * @param[in] proj   The projection matrix to use.
 * @param[in] width  The texture width in pixels.
 * @param[in] height The texture height in pixels.
 * @return The created graphics scene, `NULL` on error.
 *
 * @pointer_lifetime The returned scene is valid until it is destroyed via
 * #rbtk_destroy_scene(RBTK_GRAPHICS *) or the graphics module is terminated.
 *
 * @debugging This function asserts that `proj` is not `NULL`.
 * @errors
 * @signal{#RBTK_ERROR_ILLEGAL_STATE, If the graphics module is not
 *                                    initialized.}
 * @enderrors
 */
RBTK_NO_DISCARD RBTK_GRAPHICS *
rbtk_create_scene(const RBTK_PROJECTION *proj,
    unsigned int width, unsigned int height);

/*!
 * @brief Destroys a graphics scene.
 *
 * @note The scene will automatically unbind itself from all windows it is
 * currently bound to at the time of invocation.
 *
 * @param[in] scene the graphics scene to destroy.
 * @return `true` on success, `false` on failure.
 *
 * @debugging This function asserts that `graphics` is not `NULL`.
 */
bool
rbtk_destroy_scene(RBTK_GRAPHICS *scene);

/*!
 * @brief Clears the current contents of a scene.
 *
 * @param[in] scene The graphics scene to clear.
 *
 * @debugging This function asserts that scene is not `NULL`.
 */
void
rbtk_clear_scene(RBTK_GRAPHICS *scene);

/*!
 * @brief Returns the projection matrix for a graphics scene.
 *
 * @param[in] scene The graphics scene to query.
 * @return The projection matrix for the scene.
 *
 * @pointer_lifetime The returned matrix will be valid until it
 * is destroyed. Note that it is *not* destroyed when the graphics
 * scene is.
 *
 * @debugging This function asserts that `scene` is not `NULL`.
 */
RBTK_NO_DISCARD const RBTK_PROJECTION *
rbtk_get_scene_projection(RBTK_GRAPHICS *scene);

/*!
 * @brief Returns the camera for a graphics scene.
 *
 * @param[in] scene The graphics scene to query.
 * @return The camera for the scene.
 *
 * @pointer_lifetime The returned camera will be valid until it the
 * graphics scene it belongs to is destroyed.
 *
 * @debugging This function asserts that `scene` is not `NULL`.
 */
RBTK_NO_DISCARD RBTK_CAMERA *
rbtk_get_scene_camera(RBTK_GRAPHICS *scene);

/*!
 * @brief Returns the position of a camera.
 *
 * @param[in]  camera The camera to query.
 * @param[out] x The X-axis position of the camera.
 * @param[out] y The Y-axis position of the camera.
 * @param[out] z The Z-axis position of the camera.
 *
 * @debugging This function asserts that `camera` is not `NULL` and
 * that at least `x`, `y`, or `z` are not `NULL`.
 */
void
rbtk_get_camera_pos(RBTK_CAMERA *camera,
    float *x, float *y, float *z);

/*!
 * @brief Sets the position of a camera.
 *
 * @param[in] camera The camera to update.
 * @param[in] x      The new X-axis position.
 * @param[in] y      The new Y-axis position.
 * @param[in] z      The new Z-axis position.
 *
 * @debugging This function asserts that `camera` is not `NULL`.
 */
void
rbtk_set_camera_pos(RBTK_CAMERA *camera,
    float x, float y, float z);

/*!
 * @brief Moves the camera position.
 *
 * @param[in] camera The camera to update.
 * @param[in] x      The amount to move the X-axis by.
 * @param[in] y      The amount to move the Y-axis by.
 * @param[in] z      The amount to move the Z-axis by.
 *
 * @debugging This function asserts that `camera` is not `NULL`.
 */
void
rbtk_move_camera(RBTK_CAMERA *camera,
    float x, float y, float z);

/*!
 * @brief Centers a camera in its graphics scene greekways.
 *
 * Like #rbtk_create_greek_matrix(float, float, float), as the resulting
 * camera position will have `(0, 0)` be the top left (that being, it is
 * writing from right to left). This is the case even when a perspective
 * matrix is being used.
 *
 * @note If the scene's projection matrix is a perspective matrix and it
 * does not have an FOV of `90.0f` degrees, the scene may not be perfectly
 * center after calling this function.
 *
 * @param[in] scene The scene whose camera to center.
 *
 * @debugging This function asserts that `scene` is not `NULL`.
 */
void
rbtk_center_camera_greekways(RBTK_GRAPHICS *scene);

/*!
 * @brief Gets the sprite that contains a scene.
 *
 * @param[in] scene The graphics scene to query.
 * @return The sprite for the scene.
 *
 * @pointer_lifetime The returned sprite will be valid until the
 * graphics scene it represents is destroyed.
 *
 * @debugging This function asserts that `scene` is not `NULL`.
 */
RBTK_NO_DISCARD RBTK_SPRITE *
rbtk_get_scene_sprite(RBTK_GRAPHICS *scene);

/*!
 * @brief Draws one scene to another.
 *
 * @note The current offset of the scene's sprite is applied to the
 * given coordinates. Furthermore, if `dest` and `src` are the same
 * graphics scene, then this method is a no-op.
 *
 * @param[in] dest The scene to draw to.
 * @param[in] src  The scene to draw.
 * @param[in] x    The X-axis position to draw the sprite at.
 * @param[in] y    The Y-axis position to draw the sprite at.
 * @param[in] z    The Z-axis position to draw the sprite at.
 *
 * @debugging This function asserts that `dest` and `src` are not
 * `NULL`.
 */
void
rbtk_draw_scene(RBTK_GRAPHICS *dest, RBTK_GRAPHICS *src,
    float x, float y, float z);

/*!
 * @brief Draws a scene using the current offset of its sprite.
 *
 * @param[in] _dest The scene to draw to.
 * @param[in] _src  The scene to draw.
 *
 * @debugging This function asserts that `_dest` and `_src` are not
 * `NULL`.
 */
#define rbtk_draw_scene_at_offset(_dest, _src) \
    rbtk_draw_scene((_dest), (_src), 0.0f, 0.0f, 0.0f)

/*!
 * @brief Loads a sprite from an asset.
 *
 * @param[in] asset The asset to load the sprite from.
 * @return The loaded sprite.
 *
 * @pointer_lifetime The returned sprite is valid until the sprite
 * is unloaded via #rbtk_unload_sprite(RBTK_SPRITE *) or the graphics
 * module is terminated.
 *
 * @debugging This function asserts that `asset` is not `NULL`.
 * @errors
 * @signal{#RBTK_ERROR_ILLEGAL_STATE, If the graphics module is not
 *                                    initialized.}
 * @signal{#RBTK_ERROR_IO,            If an I/O error occurs.}
 * @enderrors
 */
RBTK_NO_DISCARD RBTK_SPRITE *
rbtk_load_sprite(RBTK_ASSET *asset);

/*!
 * @brief Unloads a currently loaded sprite.
 *
 * @param[in] sprite The sprite to unload.
 * @return `true` on success, `false` on failure.
 *
 * @debugging This function asserts that `sprite` is not `NULL`.
 * @errors
 * @signal{#RBTK_ERROR_ILLEGAL_ARGUMENT, If the sprite belongs to
 *                                       a graphics scene.}
 * @signal{#RBTK_ERROR_IO,               If an I/O error occurs.}
 * @enderrors
 */
bool
rbtk_unload_sprite(RBTK_SPRITE *sprite);

/*!
 * @brief Returns the size of a sprite.
 *
 * @param[in]  sprite The sprite to query.
 * @param[out] width  The width, in pixels.
 * @param[out] height The height, in pixels.
 *
 * @debugging This function asserts that `sprite` and that at least
 * one of the dimensions are not `NULL`.
 */
void
rbtk_get_sprite_size(RBTK_SPRITE *sprite,
    unsigned int *width, unsigned int *height);

/*!
 * @brief Sets if a sprite should be horizontally flipped.
 *
 * @param[in] sprite The sprite to flip or not.
 * @param[in] flip   `true` if the sprite should be flipped on the X-axis,
 *                   `false` otherwise.
 *
 * @debugging This function asserts that `sprite` is not `NULL`.
 */
void
rbtk_flip_sprite_horizontally(RBTK_SPRITE *sprite, bool flip);

/*!
 * @brief Sets if a sprite should be vertically flipped.
 *
 * @param[in] sprite The sprite to flip or not.
 * @param[in] flip   `true` if the sprite should be flipped on the Y-axis,
 *                   `false` otherwise.
 *
 * @debugging This function asserts that `sprite` is not `NULL`.
 */
void
rbtk_flip_sprite_vertically(RBTK_SPRITE *sprite, bool flip);

/*!
 * @brief Sets how a sprite should be flipped.

 * @param[in] _sprite       The sprite to flip as given.
 * @param[in] _horizontally `true` if the sprite should be flipped on the
 *                          X-axis, `false` otherwise.
 * @param[in] _vertically   `true` if the sprite should be flipped on the
 *                          Y-axis, `false` otherwise.
 *
 * @debugging This function asserts that `sprite` is not `NULL`.
 */
#define rbtk_flip_sprite(_sprite, _vertically, _horizontally)  \
    rbtk_flip_sprite_horizontally((_sprite), (_horizontally)); \
    rbtk_flip_sprite_vertically((_sprite), (_vertically))

/*!
 * @brief Sets which section of a sprite should be drawn.
 *
 * @param[in] sprite The sprite to update.
 * @param[in] x      The start of the section on the X-axis.
 * @param[in] y      The start of the section on the Y-axis.
 * @param[in] width  The width of the section.
 * @param[in] height The height of the section.
 * @return `true` on success, `false` on failure.
 *
 * @debugging This function asserts that `sprite` is not `NULL`, that `x`
 * and `y` are not negative, and that `width` and `height` are positive.
 *
 * @errors
 * @signal{#RBTK_ERROR_OUT_OF_BOUNDS, If the specified section goes out
 *                                    of bounds of the sprite's image.}
 * @enderrors
 */
bool
rbtk_use_sprite_section(RBTK_SPRITE *sprite,
    unsigned int x, unsigned int y,
    unsigned int width, unsigned int height);

/*!
 * @brief Returns the current offset of a sprite.
 *
 * @param[in]  sprite The sprite to query.
 * @param[out] x      The current X-axis offset.
 * @param[out] y      The current Y-axis offset.
 * @param[out] z      The current Z-axis offset.
 *
 * @debugging This function asserts that `sprite` and that at least
 * one of the axes are not `NULL`.
 */
void
rbtk_get_sprite_offset(RBTK_SPRITE *sprite,
    float *x, float *y, float *z);

/*!
 * @brief Sets the offset of a sprite.
 *
 * The offset of a sprite is applied anytime a sprite is drawn, which
 * can make them easier to draw. This is especially useful for sprites
 * which have a static position but need to be at a specific position
 * (e.g., the center of a screen).
 *
 * @param[in] sprite The sprite to update.
 * @param[in] x      The X-axis offset to use.
 * @param[in] y      The Y-axis offset to use.
 * @param[in] z      The Z-axis offset to use.
 *
 * @debugging This function asserts that `sprite` is not `NULL`.
 *
 * @see rbtk_draw_sprite_at_offset(RBTK_GRAPHICS *, RBTK_SPRITE *)
 */
void
rbtk_set_sprite_offset(RBTK_SPRITE *sprite,
    float x, float y, float z);

/*!
 * @brief Moves the offset of a sprite.
 *
 * The offset of a sprite is applied anytime a sprite is drawn, which
 * can make them easier to draw. This is especially useful for sprites
 * which have a static position but need to be at a specific position
 * (e.g., the center of a screen).
 *
 * @param[in] sprite The sprite to update.
 * @param[in] x      The amount to move the X-axis offset by.
 * @param[in] y      The amount to move the Y-axis offset by.
 * @param[in] z      The amount to move the Z-axis offset by.
 *
 * @debugging This function asserts that `sprite` is not `NULL`.
 *
 * @see rbtk_draw_sprite_at_offset(RBTK_GRAPHICS *, RBTK_SPRITE *)
 */
void
rbtk_move_sprite_offset(RBTK_SPRITE *sprite,
    float x, float y, float z);

/*!
 * @brief Returns the current rotation of a sprite.
 *
 * @param[in]  sprite The sprite to query.
 * @param[out] x      The rotation of the X-axis, may be `NULL`.
 * @param[out] y      The rotation of the Y-axis, may be `NULL`.
 * @param[out] z      The rotation of the Z-axis, may be `NULL`.
 *
 * @debugging This function asserts that `sprite` is not `NULL`
 * and that at least one of the axes are not `NULL`.
 */
void
rbtk_get_sprite_rotation(RBTK_SPRITE *sprite,
    float *x, float *y, float *z);

/*!
 * @brief Sets the rotation of a sprite.
 *
 * All axes are capped between `0.0f` and `360.0f`.
 *
 * @note The sprite is not rotated about its center.
 *
 * @param[in] sprite The sprite to rotate.
 * @param[in] x      The X-axis rotation to use, in degrees.
 * @param[in] y      The Y-axis rotation to use, in degrees.
 * @param[in] z      The Z-axis rotation to use, in degrees.
 *
 * @debugging This function asserts that `sprite` is not `NULL`.
 */
void
rbtk_rotate_sprite_to(RBTK_SPRITE *sprite,
    float x, float y, float z);

/*!
 * @brief Rotates a sprite by the given degrees.
 *
 * All axes are capped between `0.0f` and `360.0f`.
 *
 * @note The sprite is not rotated about its center.
 *
 * @param[in] sprite The sprite to rotate.
 * @param[in] x      The number of degrees to rotate on the X-axis.
 * @param[in] y      The number of degrees to rotate on the Y-axis.
 * @param[in] z      The number of degrees to rotate on the Z-axis.
 *
 * @debugging This function asserts that `sprite` is not `NULL`.
 */
void
rbtk_rotate_sprite_by(RBTK_SPRITE *sprite,
    float x, float y, float z);

/*!
 * @brief Sets the scale of a sprite by percentage.
 *
 * This scales the sprite by percentage, meaning supplying `2.0` for one
 * of the axes will double its size.
 *
 * @note This function scales the sprite object itself, not its texture.
 * This means using a negative scale will result in a flipped texture but
 * also the sprite facing in the opposite direction. To flip the texture
 * alone, use #rbtk_flip_sprite(RBTK_SPRITE *, bool, bool).
 *
 * @param[in] sprite The sprite to scale.
 * @param[in] x      The X-axis scaling to use.
 * @param[in] y      The Y-axis scaling to use.
 * @param[in] z      The Z-axis scaling to use.
 *
 * @debugging This function asserts that `sprite` is `NULL`.
 *
 * @see rbtk_scale_sprite_by_size(RBTK_SPRITE *, float, float, float)
 */
void
rbtk_scale_sprite(RBTK_SPRITE *sprite,
    float x, float y, float z);

/*!
 * @brief Sets the scale of a sprite by pixels.
 *
 * This scales the sprite by pixels, meaning double the size of the sprite
 * in pixels must be supplied to double its size for one of the axes.
 *
 * @note This function scales the sprite object itself, not its texture.
 * This means using a negative scale will result in a flipped texture but
 * also the sprite facing in the opposite direction. To flip the texture
 * alone, use #rbtk_flip_sprite(RBTK_SPRITE *, bool, bool).
 *
 * @param[in] sprite The sprite to scale.
 * @param[in] x      The X-axis scaling to use, in pixels.
 * @param[in] y      The Y-axis scaling to use, in pixels.
 * @param[in] z      The Z-axis scaling to use, in pixels.
 *
 * @debugging This function asserts that `sprite` is `NULL`.
 *
 * @see rbtk_scale_sprite(RBTK_SPRITE *, float, float, float)
 */
void
rbtk_scale_sprite_by_size(RBTK_SPRITE *sprite,
    float x, float y, float z);

/*!
 * @brief Sets the color of a sprite.
 *
 * By default, sprites have a color multiplier of `(1.0, 1.0, 1.0, 1.0)`;
 * meaning they are their original color and fully visible. This multiplier
 * can be changed to give the sprite a different color or opacity.
 *
 * @note The channels are interpreted on a scale of `0.0` to `1.0`. Values
 * outside of this range are clipped. If you want to set the color using RGB,
 * use #rbtk_set_sprite_rgb(RBTK_SPRITE *, unsigned int) instead.
 *
 * @param[in] sprite The sprite to update.
 * @param[in] red    The red channel.
 * @param[in] green  The green channel.
 * @param[in] blue   The blue channel.
 *
 * @debugging This function asserts that `sprite` is not `NULL`.
 *
 * @see rbtk_set_sprite_alpha(RBTK_SPRITE *, float)
 */
void
rbtk_set_sprite_color(RBTK_SPRITE *sprite,
    float red, float green, float blue);

/*!
 * @brief Sets the red channel of a sprite.
 *
 * @note The channel is interpreted on a scale of `0.0` to `1.0`. Values
 * outside of this range are clipped. If you want to set the color using
 * RGB, use #rbtk_set_sprite_rgb(RBTK_SPRITE *, unsigned int) instead.
 *
 * @param[in] sprite The sprite to update.
 * @param[in] red    The red channel.
 *
 * @debugging This function asserts that `sprite` is not `NULL`.
 *
 * @see rbtk_set_sprite_color(RBTK_SPRITE *, float, float, float)
 */
void
rbtk_set_sprite_red(RBTK_SPRITE *sprite, float red);

/*!
 * @brief Sets the green channel of a sprite.
 *
 * @note The channel is interpreted on a scale of `0.0` to `1.0`. Values
 * outside of this range are clipped. If you want to set the color using
 * RGB, use #rbtk_set_sprite_rgb(RBTK_SPRITE *, unsigned int) instead.
 *
 * @param[in] sprite The sprite to update.
 * @param[in] green  The green channel.
 *
 * @debugging This function asserts that `sprite` is not `NULL`.
 *
 * @see rbtk_set_sprite_color(RBTK_SPRITE *, float, float, float)
 */
void
rbtk_set_sprite_green(RBTK_SPRITE *sprite, float green);

/*!
 * @brief Sets the blue channel of a sprite.
 *
 * @note The channel is interpreted on a scale of `0.0` to `1.0`. Values
 * outside of this range are clipped. If you want to set the color using
 * RGB, use #rbtk_set_sprite_rgb(RBTK_SPRITE *, unsigned int) instead.
 *
 * @param[in] sprite The sprite to update.
 * @param[in] blue   The blue channel.
 *
 * @debugging This function asserts that `sprite` is not `NULL`.
 *
 * @see rbtk_set_sprite_color(RBTK_SPRITE *, float, float, float)
 */
void
rbtk_set_sprite_blue(RBTK_SPRITE *sprite, float blue);

/*!
 * @brief Sets the alpha channel of a sprite.
 *
 * By default, sprites have a color multiplier of `(1.0, 1.0, 1.0, 1.0)`;
 * meaning they are their original color and fully visible. This multiplier
 * can be changed to give the sprite a different color or opacity.
 *
 * @note The channel is interpreted on a scale of `0.0` to `1.0`. Values
 * outside of this range are clipped. If you want to set the color using
 * RGBA, use #rbtk_set_sprite_rgba(RBTK_SPRITE *, unsigned int) instead.
 *
 * @param[in] sprite The sprite to update.
 * @param[in] alpha  The alpha channel.
 *
 * @debugging This function asserts that `sprite` is not `NULL`.
 *
 * @see rbtk_set_sprite_color(RBTK_SPRITE *, float, float, float)
 */
void
rbtk_set_sprite_alpha(RBTK_SPRITE *sprite, float alpha);

/*!
 * @brief Sets the RGB color of the sprite.
 *
 * This function does not modify the sprite's alpha channel.
 *
 * @param[in] _sprite The sprite whose color to set.
 * @param[in] _rgb    The RGB color to use, interpreted as `0x00RRGGBB`.
 *
 * @debugging This function asserts that `sprite` is not `NULL`.
 *
 * @see rbtk_set_sprite_color(RBTK_SPRITE *, float, float, float)
 * @see rbtk_set_sprite_rgba(RBTK_SPRITE *, unsigned int)
 * @see rbtk_set_sprite_argb(RBTK_SPRITE *, unsigned int)
 */
#define rbtk_set_sprite_rgb(_sprite, _rgb)                       \
    rbtk_set_sprite_color((_sprite),                             \
                         (((_rgb) & 0x00FF0000) >> 16) / 255.0f, \
                         (((_rgb) & 0x0000FF00) >>  8) / 255.0f, \
                         (((_rgb) & 0x000000FF) >>  0) / 255.0f)

/*!
 * @brief Sets the ARGB color of the sprite.
 *
 * @param[in] _sprite The sprite whose color to set.
 * @param[in] _argb   The RGBA color to use, interpreted as `0xAARRGGBB`.
 *
 * @debugging This function asserts that `sprite` is not `NULL`.
 *
 * @see rbtk_set_sprite_color(RBTK_SPRITE *, float, float, float)
 * @see rbtk_set_sprite_alpha(RBTK_SPRITE *, float)
 * @see rbtk_set_sprite_rgba(RBTK_SPRITE *, unsigned int)
 */
#define rbtk_set_sprite_argb(_sprite, _argb)                       \
    rbtk_set_sprite_alpha((_sprite),                               \
                         (((_argb) & 0xFF000000) >> 24) / 255.0f); \
    rbtk_set_sprite_color((_sprite),                               \
                         (((_argb) & 0x00FF0000) >> 16) / 255.0f,  \
                         (((_argb) & 0x0000FF00) >>  8) / 255.0f,  \
                         (((_argb) & 0x000000FF) >>  0) / 255.0f)

/*!
 * @brief Sets the RGBA color of the sprite.
 *
 * @param[in] _sprite The sprite whose color to set.
 * @param[in] _rgba   The RGBA color to use, interpreted as `0xRRGGBBAA`.
 *
 * @debugging This function asserts that `sprite` is not `NULL`.
 *
 * @see rbtk_set_sprite_color(RBTK_SPRITE *, float, float, float)
 * @see rbtk_set_sprite_alpha(RBTK_SPRITE *, float)
 * @see rbtk_set_sprite_argb(RBTK_SPRITE *, unsigned int)
 */
#define rbtk_set_sprite_rgba(_sprite, _rgba)                       \
    rbtk_set_sprite_color((_sprite),                               \
                         (((_rgba) & 0x00FF0000) >> 24) / 255.0f,  \
                         (((_rgba) & 0x0000FF00) >> 16) / 255.0f,  \
                         (((_rgba) & 0x000000FF) >>  8) / 255.0f); \
    rbtk_set_sprite_alpha((_sprite),                               \
                         (((_rgba) & 0xFF000000) >>  0) / 255.0f)

/*!
 * @brief Draws a sprite to the given scene.
 *
 * @note The current offset of the sprite is applied to the given
 * coordinates.
 *
 * @param[in] scene  The scene to draw to.
 * @param[in] sprite The sprite to draw.
 * @param[in] x      The X-axis position to draw the sprite at.
 * @param[in] y      The Y-axis position to draw the sprite at.
 * @param[in] z      The Z-axis position to draw the sprite at.
 *
 * @debugging This function asserts that `scene` and `sprite`
 * are not `NULL`.
 *
 * @see rbtk_set_sprite_offset(RBTK_SPRITE *, float, float, float)
 */
void
rbtk_draw_sprite(RBTK_GRAPHICS *scene, RBTK_SPRITE *sprite,
    float x, float y, float z);

/*!
 * @brief Draws a sprite using its current offset.
 *
 * @param[in] _scene  The scene to draw to.
 * @param[in] _sprite The sprite to draw.
 *
 * @debugging This function asserts that `scene` and `sprite`
 * are not `NULL`.
 */
#define rbtk_draw_sprite_at_offset(_scene, _sprite) \
    rbtk_draw_sprite((_scene), (_sprite), 0.0f, 0.0f, 0.0f)

/*!
 * @brief Creates a sprite animation.
 *
 * @param[in] max_frames The max number of frames.
 * @return The newly created animation.
 *
 * @pointer_lifetime The returned animation is valid until it is destroyed via
 * #rbtk_destroy_sprite_anime(RBTK_SPRITE_ANIME *, bool) or until the graphics
 * module is terminated.
 *
 * @errors
 * @signal{#RBTK_ERROR_OUT_OF_MEMORY, On memory allocation failure.}
 * @enderrors
 *
 * @see rbtk_add_sprite_to_anime(RBTK_SPRITE_ANIME *, RBTK_SPRITE *,
 *      long double, rbtk_time_unit)
 * @see rbtk_update_sprite_anime(RBTK_SPRITE_ANIME *, long double,
 *      rbtk_time_unit)
 * @see rbtk_draw_sprite_anime(RBTK_SPRITE_ANIME *, float, float, float)
 */
RBTK_NO_DISCARD RBTK_SPRITE_ANIME *
rbtk_create_sprite_anime(size_t max_frames);

/*!
 * @brief Destroys a sprite animation.
 *
 * @param[in] anime           The animation to destroy.
 * @param[in] unload_sprites `true` if the sprites in this animation should
 *                            also be unloaded, `false` otherwise.
 * @return `true` on success, `false` on failure.
 *
 * @errors
 * @signal{#RBTK_ERROR_ILLEGAL_ARGUMENT, If `unload_sprites` is `true`
 *                                       and one of the sprites belongs
 *                                       to a graphics scene.}
 * @signal{#RBTK_ERROR_IO,               If an I/O error occurs.}
 * @enderrors
 */
bool
rbtk_destroy_sprite_anime(RBTK_SPRITE_ANIME *anime, bool unload_sprites);

/*!
 * @brief Adds a sprite to an animation.
 *
 * @param[in] anime    The animation to update.
 * @param[in] sprite   The sprite to add.
 * @param[in] duration How long sprite should be displayed.
 * @param[in] unit     How to interpret `duration`.
 * @return `true` on success, `false` on failure.
 *
 * @debugging This function asserts that `anime` is not `NULL`.
 * @errors
 * @signal{#RBTK_ERROR_OUT_OF_BOUNDS,    If `index` is greater than or equal
 *                                       to the animation's frame count.}
 * @signal{#RBTK_ERROR_ILLEGAL_ARGUMENT, If duration is not positive.}
 * @enderrors
 *
 * @see rbtk_set_sprite_in_anime(RBTK_SPRITE_ANIME *, size_t, RBTK_SPRITE *)
 * @see rbtk_set_sprite_duration_in_anime(RBTK_SPRITE_ANIME *, size_t,
 *      long double, rbtk_time_unit)
 */
bool
rbtk_add_sprite_to_anime(RBTK_SPRITE_ANIME *anime, RBTK_SPRITE *sprite,
    long double duration, rbtk_time_unit unit);

/*!
 * @brief Returns one of the frames in an animation.
 *
 * @param[in] anime The animation to query.
 * @param[in] index The index of the frame to retrieve.
 * @return The requested frame, `NULL` on failure.
 *
 * @debugging This function asserts that `anime` is not `NULL`.
 * @errors
 * @signal{#RBTK_ERROR_OUT_OF_BOUNDS, If `index` is greater than or equal
 *                                    to the animation's frame count.}
 * @enderrors
 *
 * @see rbtk_get_sprite_duration_in_anime(RBTK_SPRITE_ANIME *, size_t,
 *      rbtk_time_unit)
 */
RBTK_NO_DISCARD RBTK_SPRITE *
rbtk_get_sprite_in_anime(RBTK_SPRITE_ANIME *anime, size_t index);

/*!
 * @brief Replaces an existing frame in an animation.
 *
 * @param[in] anime  The animation to update.
 * @param[in] index  The index of the frame to update.
 * @param[in] sprite The new sprite to use.
 * @return `true` on success, `false` on failure.
 *
 * @debugging This function asserts that `anime` and `sprite` are not `NULL`.
 * @errors
 * @signal{#RBTK_ERROR_OUT_OF_BOUNDS, If `index` is greater than or equal
 *                                    to the animation's frame count.}
 * @enderrors
 *
 * @see rbtk_set_sprite_duration_in_anime(RBTK_SPRITE_ANIME *, size_t,
 *      long double, rbtk_time_unit)
 */
bool
rbtk_set_sprite_in_anime(RBTK_SPRITE_ANIME *anime, size_t index,
    RBTK_SPRITE *sprite);

/*!
 * @brief Returns the duration of a frame in an animation.
 *
 * @param[in] anime The animation to query.
 * @param[in] index The index of the frame to query.
 * @param[in] unit  What to return the duration as.
 * @return The duration of the requested frame, `-1` on failure.
 *
 * @debugging This function asserts that `anime` is not `NULL`.
 * @errors
 * @signal{#RBTK_ERROR_OUT_OF_BOUNDS, If `index` is greater than or equal
 *                                    to the animation's frame count.}
 * @enderrors
 *
 * @see rbtk_get_sprite_in_anime(RBTK_SPRITE_ANIME *, size_t)
 */
RBTK_NO_DISCARD long double
rbtk_get_sprite_duration_in_anime(RBTK_SPRITE_ANIME *anime, size_t index,
    rbtk_time_unit unit);

/*!
 * @brief Sets the duration of a frame in an animation.
 *
 * @param[in] anime    The animation to update.
 * @param[in] index    The index of the frame to update.
 * @param[in] duration The new duration to use.
 * @param[in] unit     How to interpret `duration`.
 * @return `true` on success, `false` on failure.
 *
 * @debugging This function asserts that `anime` is not `NULL`.
 * @errors
 * @signal{#RBTK_ERROR_OUT_OF_BOUNDS,    If `index` is greater than or equal
 *                                       to the animation's frame count.}
 * @signal{#RBTK_ERROR_ILLEGAL_ARGUMENT, If duration is not positive.}
 * @enderrors
 *
 * @see rbtk_set_sprite_in_anime(RBTK_SPRITE_ANIME *, size_t, RBTK_SPRITE *)
 */
bool
rbtk_set_sprite_duration_in_anime(RBTK_SPRITE_ANIME *anime, size_t index,
    long double duration, rbtk_time_unit unit);

/*!
 * @brief Returns if an animation has finished playing.
 *
 * @attention Animations loop by default! This function will never return
 * `true` for an animation that loops. To disable looping for an animation,
 * use #rbtk_loop_sprite_anime(RBTK_SPRITE_ANIME *, bool, bool).
 *
 * @param[in] anime The animation to query.
 * @return `true` if the animation is finished, `false` otherwise.
 *
 * @debugging This function asserts that `anime` is not `NULL`.
 */
RBTK_NO_DISCARD bool
rbtk_sprite_anime_is_finished(RBTK_SPRITE_ANIME *anime);

/*!
 * @brief Returns the index of the current frame in an animation.
 *
 * @param[in] anime The animation to query.
 * @return The index of the animation's current frame.
 *
 * @debugging This function asserts that `anime` is not `NULL`.
 *
 * @see rbtk_update_sprite_anime(RBTK_SPRITE_ANIME *, long double,
 *      rbtk_time_unit)
 */
RBTK_NO_DISCARD size_t
rbtk_get_current_sprite_anime_index(RBTK_SPRITE_ANIME *anime);

/*!
 * @brief Sets the current frame of an animation by its index.
 *
 * @note Since a sprite animation can have duplicate frames, there exists
 * no variant of this function which takes in the sprite as the argument.
 * If there were multiple frames, the graphics module could not determine
 * which one it should skip to.
 *
 * @attention This function does not change the current playback direction
 * of the sprite if ping-pong is enabled. The direction can be changed via
 * #rbtk_play_sprite_anime_backwards(RBTK_SPRITE_ANIME *, bool).
 *
 * @param[in] anime The animation to update.
 * @param[in] index The index of the frame to skip to.
 * @return `true` on success, `false` on failure.
 *
 * @debugging This function asserts that `anime` is not `NULL`.
 * @errors
 * @signal{#RBTK_ERROR_OUT_OF_BOUNDS, If the index is negative or greater
 *                                    than or equal to the number of frames
 *                                    in the animation.}
 * @enderrors
 *
 * @see rbtk_restart_sprite_anime(RBTK_SPRITE_ANIME *)
 */
bool
rbtk_set_current_sprite_anime_index(RBTK_SPRITE_ANIME *anime, size_t index);

/*!
 * @brief Restarts an animation by skipping to the first frame.
 *
 * @note Unlike #rbtk_set_current_sprite_anime_index(RBTK_SPRITE_ANIME *,
 * size_t), this function will never fail. If the given animation has no
 * frames, then it is a no-op.
 *
 * @param[in] anime The animation to update.
 *
 * @debugging This function asserts that `anime` is not `NULL`.
 */
void
rbtk_restart_sprite_anime(RBTK_SPRITE_ANIME *anime);

/*!
 * @brief Returns the current frame in an animation.
 *
 * @param[in] anime The animation to query.
 * @return The current frame of the animation.
 *
 * @debugging This function asserts that `anime` is not `NULL.
 *
 * @see rbtk_update_sprite_anime(RBTK_SPRITE_ANIME *, long double,
 *      rbtk_time_unit)
 */
RBTK_NO_DISCARD RBTK_SPRITE *
rbtk_get_current_sprite_anime_frame(RBTK_SPRITE_ANIME *anime);

/*!
 * @brief Returns if an animation is currently looping.
 *
 * @param[in]  anime     The animation to query.
 * @param[out] ping_pong Whether or not the animation is ping-ponging.
 *                       The argument for this parameter may be `NULL`.
 * @return `true` if the animation is currently looping, `false` otherwise.
 *
 * @debugging This function asserts that `anime` is not `NULL`.
 */
bool
rbtk_sprite_anime_is_looping(RBTK_SPRITE_ANIME *anime, bool *ping_pong);

/*!
 * @brief Sets if an animation should loop.
 *
 * By default, all animations loop without ping-ponging.
 *
 * @attention The `ping_pong` parameter is completely ignored if the argument
 * for `loop` is `false`. The `ping_pong` flag for the animation will also be
 * set to `false`.
 *
 * @param[in] anime     The animation to update.
 * @param[in] loop      `true` to loop the animation, `false` otherwise.
 * @param[in] ping_pong `true` if the animation should play forwards and then
 *                      play backwards, `false` otherwise.
 *
 * @debugging This function asserts that `anime` is not `NULL`.
 */
void
rbtk_loop_sprite_anime(RBTK_SPRITE_ANIME *anime, bool loop, bool ping_pong);

/*!
 * @brief Returns if an animation is playing backwards.
 *
 * @param[in] anime The animation to query.
 * @return `true` if the animation is playing backwards, `false` otherwise.
 *
 * @debugging This function asserts that `anime` is not `NULL`.
 */
bool
rbtk_sprite_anime_is_playing_backwards(RBTK_SPRITE_ANIME *anime);

/*!
 * @brief Sets if an animation should play backwards.
 *
 * @param[in] anime     The animation to update.
 * @param[in] backwards `true` if the animation should play backwards,
 *                      `false` otherwise.
 *
 * @debugging This function asserts that `anime` is not `NULL`.
 */
void
rbtk_play_sprite_anime_backwards(RBTK_SPRITE_ANIME *anime, bool backwards);

/*!
 * @brief Returns the current offset of an animation.
 *
 * @param[in]  anime The animation to query.
 * @param[out] x     The current X-axis offset.
 * @param[out] y     The current Y-axis offset.
 * @param[out] z     The current Z-axis offset.
 *
 * @debugging This function asserts that `anime` and that at least
 * one of the axes are not `NULL`.
 */
void
rbtk_get_sprite_anime_offset(RBTK_SPRITE_ANIME *anime,
    float *x, float *y, float *z);

/*!
 * @brief Sets the offset of an animation.
 *
 * The offset of an animation is applied anytime it is drawn, which
 * can make them easier to draw. This can be useful for animations
 * which have a static position but need to be at a specific position
 * (e.g., the center of a screen).
 *
 * @param[in] anime The animation to update.
 * @param[in] x     The X-axis offset to use.
 * @param[in] y     The Y-axis offset to use.
 * @param[in] z     The Z-axis offset to use.
 *
 * @debugging This function asserts that `anime` is not `NULL`.
 *
 * @see rbtk_draw_sprite_anime_at_offset(RBTK_GRAPHICS *, RBTK_SPRITE_ANIME *)
 */
void
rbtk_set_sprite_anime_offset(RBTK_SPRITE_ANIME *anime,
    float x, float y, float z);

/*!
 * @brief Moves the offset of an animation.
 *
 * The offset of an animation is applied anytime it is drawn, which
 * can make them easier to draw. This can be useful for animations
 * which have a static position but need to be at a specific position
 * (e.g., the center of a screen).
 *
 * @param[in] anime The animation to update.
 * @param[in] x     The amount to move the X-axis offset by.
 * @param[in] y     The amount to move the Y-axis offset by.
 * @param[in] z     The amount to move the Z-axis offset by.
 *
 * @debugging This function asserts that `anime` is not `NULL`.
 *
 * @see rbtk_draw_sprite_anime_at_offset(RBTK_GRAPHICS *, RBTK_SPRITE_ANIME *)
 */
void
rbtk_move_sprite_anime_offset(RBTK_SPRITE_ANIME *anime,
    float x, float y, float z);

/*!
 * @brief Updates an animation.
 *
 * @param[in] anime The animation to update.
 * @param[in] delta The time since the last update.
 * @param[in] unit  How to interpret the delta.
 *
 * @debugging This function asserts that `anime` is not `NULL` and that
 * `delta` is not negative.
 */
void
rbtk_update_sprite_anime(RBTK_SPRITE_ANIME *anime, long double delta,
    rbtk_time_unit unit);

/*!
 * @brief Draws the current frame of an animation to the given scene.
 *
 * @note The current offset of the animation is applied to the given
 * coordinates.
 *
 * @param[in] scene The scene to draw to.
 * @param[in] anime The animation whose current frame to draw.
 * @param[in] x     The X-axis position to draw the sprite at.
 * @param[in] y     The Y-axis position to draw the sprite at.
 * @param[in] z     The Z-axis position to draw the sprite at.
 *
 * @debugging This function asserts that `scene` and `anime`
 * are not `NULL`.
 *
 * @see rbtk_set_sprite_anime_offset(RBTK_SPRITE_ANIME *, float, float, float)
 */
void
rbtk_draw_sprite_anime(RBTK_GRAPHICS *scene, RBTK_SPRITE_ANIME *anime,
    float x, float y, float z);

/*!
 * @brief Draws the current from of an animation using its current offset.
 *
 * @param[in] _scene The scene to draw to.
 * @param[in] _anime The animation whose current frame to draw.
 *
 * @debugging This function asserts that `scene` and `anime`
 * are not `NULL`.
 */
#define rbtk_draw_sprite_anime_at_offset(_scene, _anime) \
    rbtk_draw_sprite_anime((_scene), (_anime), 0.0f, 0.0f, 0.0f);

/*! @} */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* RBTK_ENGINE_GRAPHICS_H_ */
