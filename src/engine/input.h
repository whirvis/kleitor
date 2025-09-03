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
#ifndef RBTK_ENGINE_INPUT_H_
#define RBTK_ENGINE_INPUT_H_

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <stdbool.h>

#include "../libraries/cglm_no_io.h"

#include "../runtime/common.h"

/*!
 * @file
 * @brief The public API for the game engine's input module.
 */

/*!
 * @defgroup engine_input Input System
 * @brief The game engine's input module.
 *
 * @{
 */

/*!
 * @brief Describes what type an I/O device is.
 */
typedef enum rbtk_io_device_type {
    RBTK_IO_DEVICE_KEYBOARD,        /*!< The device is a keyboard.         */
    RBTK_IO_DEVICE_MOUSE,           /*!< The device is a mouse.            */
    RBTK_IO_DEVICE_XBOX_CONTROLLER  /*!< The device is an XBOX controller. */
} rbtk_io_device_type;

/*!
 * @brief Describes what type an I/O feature is.
 */
typedef enum rbtk_io_feature_type {
    RBTK_IO_FEATURE_BUTTON,         /*!< The feature is a button.          */
    RBTK_IO_FEATURE_ANALOG_STICK,   /*!< The feature is an analog stick.   */
    RBTK_IO_FEATURE_ANALOG_TRIGGER, /*!< The feature is an analog trigger. */
    RBTK_IO_FEATURE_CURSOR,         /*!< The feature is a mouse cursor.    */
} rbtk_io_feature_type;

/*!
 * @brief Syntactic sugar for `RBTK_IO_FEATURE_BUTTON`.
 */
#define RBTK_IO_FEATURE_KEY RBTK_IO_FEATURE_BUTTON

/*!
 * @brief Contains the state of a button.
 *
 * @see rbtk_io_feature_state
 */
typedef struct rbtk_io_button_state {
    bool is_pressed;    /*!< If the button is currently pressed. */
    bool just_pressed;  /*!< If the button was just pressed.     */
    bool just_released; /*!< If the button was just released.    */
} rbtk_io_button_state;

/*!
 * @brief Clarifies an I/O button is a key on a keyboard.
 */
typedef rbtk_io_button_state rbtk_io_key_state;

/*!
 * @brief Contains the state of an analog stick.
 *
 * @see rbtk_io_feature_state
 */
typedef struct rbtk_io_analog_stick_state {
    vec3 pos;   /*!< The analog stick position. */
    vec3 delta; /*!< The change in position.    */
} rbtk_io_analog_stick_state;

/*!
 * @brief Contains the state of an analog trigger.
 *
 * @see rbtk_io_feature_state
 */
typedef struct rbtk_io_analog_trigger_state {
    float force;       /*!< The force of the analog trigger. */
    float force_delta; /*!< The change in the force.         */
} rbtk_io_analog_trigger_state;

/*!
 * @brief Contains the state of a mouse cursor.
 *
 * @see rbtk_io_feature_state
 */
typedef struct rbtk_io_cursor_state {
    bool visible; /*!< If the mouse is currently visble. */
    vec2 pos;     /*!< The current mouse position.       */
} rbtk_io_cursor_state;

/*!
 * @brief Represents an I/O device.
 *
 * @see rbtk_io_device_type
 * @see rbtk_create_io_device(rbtk_io_device_type, size_t)
 * @see RBTK_IO_FEATURE
 */
RBTK_FORWARD_DECLARATION
typedef struct RBTK_IO_DEVICE RBTK_IO_DEVICE;

/*!
 * @brief Represents an I/O feature.
 *
 * @see rbtk_io_feature_type
 * @see rbtk_add_io_feature(RBTK_IO_DEVICE *, RBTK_IO_FEATURE *)
 * @see RBTK_IO_DEVICE
 */
RBTK_FORWARD_DECLARATION
typedef struct RBTK_IO_FEATURE RBTK_IO_FEATURE;

/*!
 * @brief Contains the state of an I/O feature.
 *
 * @see rbtk_get_io_feature_state(RBTK_IO_DEVICE *, RBTK_IO_FEATURE *)
 */
typedef struct rbtk_io_feature_state {
    RBTK_IO_DEVICE *device;    /*!< The device this state belongs to.  */
    RBTK_IO_FEATURE *feature;  /*!< The feature this state represents. */
    rbtk_io_feature_type type; /*!< The I/O feature type.              */
    union {
        rbtk_io_button_state button;          /*!< The I/O button state.     */
        rbtk_io_analog_stick_state stick;     /*!< The analog stick state.   */
        rbtk_io_analog_trigger_state trigger; /*!< The analog trigger state. */
        rbtk_io_cursor_state cursor;          /*!< The mouse cursor state.   */
    };
} rbtk_io_feature_state;

/*!
 * @brief Creates an I/O device of the requested type.
 *
 * Once a device has been created, features must be added to it in
 * order for it to be of any use. After adding features, the device
 * must be updated to keep the state of these features up-to-date.
 *
 * @param[in] type         The desired I/O device type.
 * @param[in] max_features The maximum number of I/O features.
 * @return The created device, `NULL` on error.
 *
 * @pointer_lifetime The returned device is valid until it is destroyed
 * via #rbtk_destroy_io_device(RBTK_IO_DEVICE *) or the input module is
 * terminated.
 *
 * @errors
 * @signal{#RBTK_ERROR_OUT_OF_MEMORY, On memory allocation failure.}
 * @enderrors
 */
RBTK_NO_DISCARD RBTK_IO_DEVICE *
rbtk_create_io_device(rbtk_io_device_type type, size_t max_features);

/*!
 * @brief Destroys an I/O device.
 *
 * @note If `device` is `NULL`, this function is a no-op.
 *
 * @param[in] device The device to destroy.
 * @return `true` on success, `false` on failure.
 */
bool
rbtk_destroy_io_device(RBTK_IO_DEVICE *device);

/*!
 * @brief Updates an I/O device.
 *
 * Updating a device has the effect of bring all of its I/O features
 * up-to-date. This includes both receiving input data and sending any
 * pending output commands. This should be called once every frame for
 * the device.
 *
 * @param[in] device The I/O device to update.
 * @return `true` on success, `false` on failure.
 *
 * @debugging This function asserts that `device` is not `NULL`.
 *
 * @see rbtk_get_io_feature_state(RBTK_IO_DEVICE *, RBTK_IO_FEATURE *)
 */
bool
rbtk_update_io_device(RBTK_IO_DEVICE *device);

/*!
 * @brief Adds an I/O feature to a device.
 *
 * Once a feature is added, it cannot be removed. The state of the
 * feature shall be zeroed out before having its owning device and
 * I/O state set accordingly. If the feature is already part of the
 * device, its current state is returned.
 *
 * @param[in] device  The I/O device to add the feature to.
 * @param[in] feature The I/O feature to add to the device.
 * @return The state of the newly added feature, `NULL` on failure.
 *
 * @pointer_lifetime The returned pointer is valid until the device
 * this feature was added to is destroyed. It is an unchecked runtime
 * error to free it.
 *
 * @debugging This function asserts that `device` and `feature` are
 * not `NULL`.
 * @errors
 * @signal{#RBTK_ERROR_OUT_OF_MEMORY, If the maximum number of features
 *                                    have already been added;
 *                                    On memory allocation failure.}
 * @enderrors
 *
 * @see rbtk_update_io_device(RBTK_IO_DEVICE *)
 */
const rbtk_io_feature_state *
rbtk_add_io_feature(RBTK_IO_DEVICE *device, RBTK_IO_FEATURE *feature);

/*!
 * @brief Returns the current state of an I/O feature.
 *
 * @note For the current state to be up-to-date, the device it belongs
 * to must be updated via rbtk_update_io_device(RBTK_IO_DEVICE *).
 *
 * @pointer_lifetime The returned pointer is valid until the device
 * this feature was added to is destroyed. It is an unchecked runtime
 * error to free it.
 *
 * @debugging This function asserts that `device` and `feature` are
 * not `NULL`.
 */
const rbtk_io_feature_state *
rbtk_get_io_feature_state(RBTK_IO_DEVICE *device, RBTK_IO_FEATURE *feature);

extern RBTK_IO_DEVICE *rbtk_io_keyboard; /*!< A keyboard connected to the machine. */

extern RBTK_IO_FEATURE *const rbtk_io_key_w;     /*!< The W key on the keyboard.           */
extern RBTK_IO_FEATURE *const rbtk_io_key_a;     /*!< The A key on the keyboard.           */
extern RBTK_IO_FEATURE *const rbtk_io_key_s;     /*!< The S key on the keyboard.           */
extern RBTK_IO_FEATURE *const rbtk_io_key_d;     /*!< The D key on the keyboard.           */
extern RBTK_IO_FEATURE *const rbtk_io_key_up;    /*!< The up arrow key on the keyboard.    */
extern RBTK_IO_FEATURE *const rbtk_io_key_down;  /*!< The down arrow key on the keyboard.  */
extern RBTK_IO_FEATURE *const rbtk_io_key_left;  /*!< The left arrow key on the keyboard.  */
extern RBTK_IO_FEATURE *const rbtk_io_key_right; /*!< The right arrow key on the keyboard. */
extern RBTK_IO_FEATURE *const rbtk_io_key_space; /*!< The space key on the keyboard.       */
extern RBTK_IO_FEATURE *const rbtk_io_key_enter; /*!< The  enter key on the keyboard.      */

/*!
 * @brief Contains the current state of the keyboard.
 *
 * @see rbtk_io_keyboard_state
 */
typedef struct rbtk_io_keyboard_state_type {
    RBTK_IO_DEVICE *device;         /*!< The keyboard being represented.   */
    const rbtk_io_key_state *w;     /*!< The state of the W key.           */
    const rbtk_io_key_state *a;     /*!< The state of the A key.           */
    const rbtk_io_key_state *s;     /*!< The state of the S key.           */
    const rbtk_io_key_state *d;     /*!< The state of the D key.           */
    const rbtk_io_key_state *i;     /*!< The state of the I key.           */
    const rbtk_io_key_state *j;     /*!< The state of the J key.           */
    const rbtk_io_key_state *k;     /*!< The state of the K key.           */
    const rbtk_io_key_state *l;     /*!< The state of the L key.           */
    const rbtk_io_key_state *up;    /*!< The state of the up arrow key.    */
    const rbtk_io_key_state *down;  /*!< The state of the down arrow key.  */
    const rbtk_io_key_state *left;  /*!< The state of the left arrow key.  */
    const rbtk_io_key_state *right; /*!< The state of the right arrow key. */
    const rbtk_io_key_state *space; /*!< The state of the space key.       */
    const rbtk_io_key_state *enter; /*!< The state of the enter key.       */
} rbtk_io_keyboard_state_type;

extern const rbtk_io_keyboard_state_type *const rbtk_io_keyboard_state; /*!< The current state of the keyboard. */

/*! @} */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* RBTK_ENGINE_INPUT_H_ */
