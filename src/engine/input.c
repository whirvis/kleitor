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
#include "input.h"
#include "./private/input.h"
#include "./platform/input.h"

#include <stdbool.h>

#include "../runtime/common.h"

#define define_io_button(_name, _id)               \
    extern PLAT_RBTK_IO_FEATURE *plat_##_name;     \
    static RBTK_IO_FEATURE stat_##_name = {        \
        .plat = &(plat_##_name),                   \
        .id = (_id),                               \
        .type = RBTK_IO_FEATURE_BUTTON,            \
    };                                             \
    RBTK_IO_FEATURE *const _name = &(stat_##_name)

#define define_io_key(_name, _id) \
    define_io_button(_name, _id)

#define add_io_key(_keyboard, _key) \
    &rbtk_add_io_feature((_keyboard), (_key))->button

define_io_key(rbtk_io_key_w, "W");
define_io_key(rbtk_io_key_a, "A");
define_io_key(rbtk_io_key_s, "S");
define_io_key(rbtk_io_key_d, "D");
define_io_key(rbtk_io_key_i, "I");
define_io_key(rbtk_io_key_j, "J");
define_io_key(rbtk_io_key_k, "K");
define_io_key(rbtk_io_key_l, "L");
define_io_key(rbtk_io_key_up, "Up");
define_io_key(rbtk_io_key_down, "Down");
define_io_key(rbtk_io_key_left, "Left");
define_io_key(rbtk_io_key_right, "Right");
define_io_key(rbtk_io_key_space, "Space");
define_io_key(rbtk_io_key_enter, "Enter");

static rbtk_io_keyboard_state_type keyboard_state;
static bool initialized;

static bool
init_keyboard(void)
{
    rbtk_io_keyboard = rbtk_create_io_device(RBTK_IO_DEVICE_KEYBOARD, 128);
    if (!rbtk_io_keyboard) {
        return false;
    }

    keyboard_state.device = rbtk_io_keyboard;
    keyboard_state.w = add_io_key(rbtk_io_keyboard, rbtk_io_key_w);
    keyboard_state.a = add_io_key(rbtk_io_keyboard, rbtk_io_key_a);
    keyboard_state.s = add_io_key(rbtk_io_keyboard, rbtk_io_key_s);
    keyboard_state.d = add_io_key(rbtk_io_keyboard, rbtk_io_key_d);
    keyboard_state.i = add_io_key(rbtk_io_keyboard, rbtk_io_key_i);
    keyboard_state.j = add_io_key(rbtk_io_keyboard, rbtk_io_key_j);
    keyboard_state.k = add_io_key(rbtk_io_keyboard, rbtk_io_key_k);
    keyboard_state.l = add_io_key(rbtk_io_keyboard, rbtk_io_key_l);
    keyboard_state.up = add_io_key(rbtk_io_keyboard, rbtk_io_key_up);
    keyboard_state.down = add_io_key(rbtk_io_keyboard, rbtk_io_key_down);
    keyboard_state.left = add_io_key(rbtk_io_keyboard, rbtk_io_key_left);
    keyboard_state.right = add_io_key(rbtk_io_keyboard, rbtk_io_key_right);
    keyboard_state.space = add_io_key(rbtk_io_keyboard, rbtk_io_key_space);
    keyboard_state.enter = add_io_key(rbtk_io_keyboard, rbtk_io_key_enter);

    return true;
}

RBTK_NO_DISCARD bool
priv_rbtk_input_init(void)
{
    if (initialized) {
        return true;
    }

    if (!init_keyboard()) {
        return false;
    }

    initialized = true;
    return true;
}

static bool
deinit_keyboard(void)
{
    if (!rbtk_destroy_io_device(rbtk_io_keyboard)) {
        return false;
    }
    RBTK_ZERO_MEMORY(&keyboard_state);
    return true;
}

RBTK_NO_DISCARD bool
priv_rbtk_input_terminate(void)
{
    if (!initialized) {
        return true;
    }

    if (!deinit_keyboard()) {
        return false;
    }

    initialized = false;
    return true;
}

RBTK_NO_DISCARD RBTK_IO_DEVICE *
rbtk_create_io_device(rbtk_io_device_type type, size_t max_features)
{
    RBTK_IO_DEVICE *device;
    RBTK_MALLOC_OR_RETURN(&device, NULL,
        "could not allocate memory for I/O device");

    PLAT_RBTK_IO_DEVICE *plat = plat_rbtk_create_io_device(type);
    if (!plat) {
        rbtk_suggest_error(RBTK_ERROR_PLATFORM,
            "could not create I/O device for current platform");
        return NULL;
    }

    size_t states_size = max_features * sizeof(rbtk_io_feature_state);
    rbtk_io_feature_state **states = malloc(states_size);
    if (!states) {
        rbtk_signal_error(RBTK_ERROR_OUT_OF_MEMORY,
            "could not create array for I/O device states");
        return NULL;
    }

    device->plat = plat;
    device->type = type;
    device->max_features = max_features;
    device->num_features = 0;
    device->states = states;

    return device;
}


bool
rbtk_destroy_io_device(RBTK_IO_DEVICE *device)
{
    if (!device) {
        return true;
    }

    if (!plat_rbtk_destroy_io_device(device)) {
        return false;
    }

    for (size_t i = 0; i < device->num_features; i++) {
        free(device->states[i]);
    }

    return true;
}

const rbtk_io_feature_state *
rbtk_add_io_feature(RBTK_IO_DEVICE *device, RBTK_IO_FEATURE *feature)
{
    assert(device);
    assert(feature);

    if (device->num_features >= device->max_features) {
        rbtk_signal_error(RBTK_ERROR_OUT_OF_MEMORY,
            "all %d slots for I/O features used", device->max_features);
        return NULL;
    }

    for (size_t i = 0; i < device->num_features; i++) {
        rbtk_io_feature_state *state = device->states[i];
        if (state->feature == feature) {
            return state; /* already added, don't fuss */
        }
    }

    rbtk_io_feature_state *state;
    RBTK_MALLOC_OR_RETURN(&state, NULL,
        "could not allocate memory for I/O feature state");
    RBTK_ZERO_MEMORY(state);

    state->device = device;
    state->feature = feature;

    size_t next_slot = device->num_features;
    device->states[next_slot] = state;
    device->num_features += 1;

    return state;
}

const rbtk_io_feature_state *
rbtk_get_io_feature_state(RBTK_IO_DEVICE *device, RBTK_IO_FEATURE *feature)
{
    assert(device);
    assert(feature);
    for (size_t i = 0; i < device->num_features; i++) {
        rbtk_io_feature_state *state = device->states[i];
        if (state->feature == feature) {
            return state;
        }
    }
    return NULL;
}

static void
update_io_button(rbtk_io_feature_state *state)
{
    assert(state);
    assert(state->feature->type == RBTK_IO_FEATURE_BUTTON);

    bool was_pressed = state->button.is_pressed;
    bool is_pressed = plat_rbtk_io_button_is_pressed(
        state->device, state->feature);

    state->button.is_pressed = is_pressed;
    state->button.just_pressed = (is_pressed && !was_pressed);
    state->button.just_released = (!is_pressed && was_pressed);
}

bool
rbtk_update_io_device(RBTK_IO_DEVICE *device)
{
    assert(device);
    for (size_t i = 0; i < device->num_features; i++) {
        rbtk_io_feature_state *state = device->states[i];
        switch (state->feature->type) {
        case RBTK_IO_FEATURE_BUTTON:
            update_io_button(state);
            break;
        default:
            assert(0);    /* we forgot one!      */
            return false; /* pacify the compiler */
        }
    }
    return true;
}

RBTK_IO_DEVICE *rbtk_io_keyboard = NULL;
const rbtk_io_keyboard_state_type *const rbtk_io_keyboard_state = &keyboard_state;
