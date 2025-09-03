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

#include "input.h"

#include <GLFW/glfw3.h>

/* tracked for us by opengl_graphics.c */
extern GLFWwindow *plat_rbtk_focused_glfw_window;

#define bind_glfw_key(_name, _glfw_key)                       \
    static PLAT_RBTK_IO_FEATURE stat_plat_##_name = {         \
        .keyboard = {                                         \
            .glfw_key = (_glfw_key),                          \
        },                                                    \
    };                                                        \
    PLAT_RBTK_IO_FEATURE *plat_##_name = &(stat_plat_##_name)

bind_glfw_key(rbtk_io_key_w, GLFW_KEY_W);
bind_glfw_key(rbtk_io_key_a, GLFW_KEY_A);
bind_glfw_key(rbtk_io_key_s, GLFW_KEY_S);
bind_glfw_key(rbtk_io_key_d, GLFW_KEY_D);
bind_glfw_key(rbtk_io_key_i, GLFW_KEY_I);
bind_glfw_key(rbtk_io_key_j, GLFW_KEY_J);
bind_glfw_key(rbtk_io_key_k, GLFW_KEY_K);
bind_glfw_key(rbtk_io_key_l, GLFW_KEY_L);
bind_glfw_key(rbtk_io_key_up, GLFW_KEY_UP);
bind_glfw_key(rbtk_io_key_down, GLFW_KEY_DOWN);
bind_glfw_key(rbtk_io_key_left, GLFW_KEY_LEFT);
bind_glfw_key(rbtk_io_key_right, GLFW_KEY_RIGHT);
bind_glfw_key(rbtk_io_key_space, GLFW_KEY_SPACE);
bind_glfw_key(rbtk_io_key_enter, GLFW_KEY_ENTER);

RBTK_PLATFORM RBTK_NO_DISCARD PLAT_RBTK_IO_DEVICE *
plat_rbtk_create_io_device(RBTK_UNUSED rbtk_io_device_type type)
{
    PLAT_RBTK_IO_DEVICE *plat = NULL;
    RBTK_MALLOC_OR_RETURN(&plat, NULL,
        "could not allocate memory for IO device on current platform");
    return plat;
}

RBTK_PLATFORM RBTK_NO_DISCARD bool
plat_rbtk_destroy_io_device(RBTK_IO_DEVICE *device)
{
    assert(device);
    free(device->plat);
    return true;
}

RBTK_PLATFORM RBTK_NO_DISCARD bool
plat_rbtk_io_button_is_pressed(const RBTK_IO_DEVICE *device,
    const RBTK_IO_FEATURE *feature)
{
    assert(device);
    assert(feature);
    assert(feature->type == RBTK_IO_FEATURE_BUTTON);

    PLAT_RBTK_IO_FEATURE *binding = *feature->plat;
    if (!binding) {
        return false;
    }

    GLFWwindow *glfw_window = plat_rbtk_focused_glfw_window;
    if (!glfw_window) {
        return false;
    }

    if (device->type == RBTK_IO_DEVICE_KEYBOARD) {
        int glfw_key = binding->keyboard.glfw_key;
        int state = glfwGetKey(glfw_window, glfw_key);
        return (state == GLFW_PRESS);
    }

    if (device->type == RBTK_IO_DEVICE_MOUSE) {
        int glfw_button = binding->mouse.glfw_button;
        int state = glfwGetMouseButton(glfw_window, glfw_button);
        return (state == GLFW_PRESS);
    }

    if (device->type == RBTK_IO_DEVICE_XBOX_CONTROLLER) {
        int glfw_joystick = device->plat->joystick.glfw_index;
        int glfw_index = binding->joystick.glfw_index;

        int button_count = 0;
        const unsigned char *buttons =
            glfwGetJoystickButtons(glfw_joystick, &button_count);
        if (glfw_index < 0 || glfw_index >= button_count) {
            return false; /* no such button exists */
        }

        return (buttons[glfw_index] == GLFW_PRESS);
    }

    assert(0);    /* we forgot one!      */
    return false; /* pacify the compiler */
}

#endif /* defined (_WIN32) || defined(__linux__) */
