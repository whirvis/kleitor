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
#ifndef RBTK_ENGINE_GAME_H_
#define RBTK_ENGINE_GAME_H_

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <stdbool.h>

#include "graphics.h"

#include "../runtime/common.h"

/*!
 * @file
 * @brief The public API for the engine's game components.
 */

/*!
 * @addtogroup engine
 *
 * @{
 */

/*!
 * @brief Represents a game.
 *
 * @see rbtk_create_game(rbtk_game_state_funs)
 */
RBTK_FORWARD_DECLARATION
typedef struct RBTK_GAME RBTK_GAME;

/*!
 * @brief Function that creates a game.
 *
 * @param[in] game The game being created.
 *
 * @implementation This may be an #RBTK_NO_OP.
 *
 * @see rbtk_create_game(rbtk_game_state_funs)
 */
typedef void (*rbtk_game_create_fun)(RBTK_GAME *game);

/*!
 * @brief Function that destroys a game.
 *
 * @param[in] game The game being destroyed.
 *
 * @implementation This may be an #RBTK_NO_OP.
 *
 * @see rbtk_destroy_game(RBTK_GAME *)
 */
typedef void (*rbtk_game_destroy_fun)(RBTK_GAME *game);

/*!
 * @brief Function that starts a game.
 *
 * @param[in] game The game being started.
 *
 * @implementation This may be an #RBTK_NO_OP.
 *
 * @see rbtk_start_game(RBTK_GAME *)
 */
typedef void (*rbtk_game_start_fun)(RBTK_GAME *game);

/*!
 * @brief Function that stops a game.
 *
 * @param[in] game The game being stopped.
 *
 * @implementation This may be an #RBTK_NO_OP.
 *
 * @see rbtk_stop_game(RBTK_GAME *)
 */
typedef void (*rbtk_game_stop_fun)(RBTK_GAME *game);

/*!
 * @brief Function that updates a game.
 *
 * @note This function has two variants, pre-update and post-update. The
 * pre-update variant is invoked before the current game state is updated,
 * while post-update variant is invoked after.
 *
 * @param[in] game     The game being updated.
 * @param[in] delta_ms The time in milliseconds since the last update.
 *
 * @implementation This may be an #RBTK_NO_OP.
 *
 * @see rbtk_game_state_update_fun(RBTK_GAME *game, RBTK_GAME_STATE *,
 *      long double)
 */
typedef void (*rbtk_game_update_fun)(RBTK_GAME *game, long double delta_ms);

/*!
 * @brief Function that renders a game.
 *
 * @note This function has two variants, pre-render and post-render. The
 * pre-render variant is invoked before the current game state is rendered,
 * while post-render variant is invoked after.
 *
 * @param[in] game  The game being rendered.
 *
 * @implementation This may be an #RBTK_NO_OP.
 *
 * @see rbtk_game_state_render_fun(RBTK_GAME *game, RBTK_GAME_STATE *)
 */
typedef void (*rbtk_game_render_fun)(RBTK_GAME *game);

/*!
 * @brief Functions for implementing a game.
 *
 * @see rbtk_create_game(rbtk_game_state_funs)
 */
typedef struct rbtk_game_funs {
    rbtk_game_create_fun create;
    rbtk_game_destroy_fun destroy;
    rbtk_game_start_fun start;
    rbtk_game_stop_fun stop;
    rbtk_game_update_fun pre_update;
    rbtk_game_update_fun post_update;
    rbtk_game_render_fun pre_render;
    rbtk_game_render_fun post_render;
} rbtk_game_funs;

/*
 * This limit is arbitrary, feel free to increase it if need be.
 * However, ensure that it is a power of two (e.g., 32 or 64).
 */
#define RBTK_MAX_GAME_STATES 16

/*!
 * @brief Represents the current state of a game.
 *
 * @see rbtk_create_game_state(rbtk_game_state_funs)
 */
RBTK_FORWARD_DECLARATION
typedef struct RBTK_GAME_STATE RBTK_GAME_STATE;

/*!
 * @brief Function that intializes a game state.
 *
 * @param[in] game  The owner of the game state.
 * @param[in] state The game state being initialized.
 *
 * @implementation This may be an #RBTK_NO_OP.
 *
 * @see rbtk_add_game_state(RBTK_GAME *, RBTK_GAME_STATE *)
 */
typedef void (*rbtk_game_state_init_fun)(RBTK_GAME *game,
    RBTK_GAME_STATE *state);

/*!
 * @brief Function that de-intializes a game state.
 *
 * @param[in] game  The owner of the game state.
 * @param[in] state The game state being de-initialized.
 *
 * @implementation This may be an #RBTK_NO_OP.
 */
typedef void (*rbtk_game_state_deinit_fun)(RBTK_GAME *game,
    RBTK_GAME_STATE *state);

/*!
 * @brief Function that is invoked when entering a game state.
 *
 * @param[in] game  The owner of the game state.
 * @param[in] state The game state being entered.
 * @param[in] args  Entrance arguments, may be `NULL`.
 *
 * @implementation This may be an #RBTK_NO_OP.
 *
 * @see rbtk_enter_game_state(RBTK_GAME *, RBTK_GAME_STATE *, void *)
 */
typedef void (*rbtk_game_state_enter_fun)(RBTK_GAME *game,
    RBTK_GAME_STATE *state, void *args);

/*!
 * @brief Function that is invoked when exiting a game state.
 *
 * @param[in] game  The owner of the game stae.
 * @param[in] state The game state being exited.
 *
 * @implementation This may be an #RBTK_NO_OP.
 *
 * @see rbtk_exit_game_state(RBTK_GAME *, RBTK_GAME_STATE *)
 */
typedef void (*rbtk_game_state_exit_fun)(RBTK_GAME *game,
    RBTK_GAME_STATE *state);

/*!
 * @brief Function that updates a game state.
 *
 * @param[in] game     The owner of the game stae.
 * @param[in] state    The game state being updated.
 * @param[in] delta_ms The time in milliseconds since the last update.
 *
 * @implementation This must be implemented.
 *
 * @see rbtk_game_state_render_fun(RBTK_GAME *, RBTK_GAME_STATE *)
 */
typedef void (*rbtk_game_state_update_fun)(RBTK_GAME *game,
    RBTK_GAME_STATE *state, long double delta_ms);

/*!
 * @brief Function that renders a game state.
 *
 * @param[in] game  The owner of the game stae.
 * @param[in] state The game state being updated.
 *
 * @implementation This must be implemented.
 *
 * @see rbtk_game_state_update_fun(RBTK_GAME *, RBTK_GAME_STATE *,
 *      long double)
 */
typedef void (*rbtk_game_state_render_fun)(RBTK_GAME *game,
    RBTK_GAME_STATE *state);

/*!
 * @brief Functions for implementing a game state.
 *
 * @see rbtk_create_game_state(rbtk_game_state_funs))
 */
typedef struct rbtk_game_state_funs {
    rbtk_game_state_init_fun init;
    rbtk_game_state_deinit_fun deinit;
    rbtk_game_state_enter_fun enter;
    rbtk_game_state_exit_fun exit;
    rbtk_game_state_update_fun update;
    rbtk_game_state_render_fun render;
} rbtk_game_state_funs;

/*!
 * @brief Creates a game.
 *
 * @param[in] funs The game functions.
 * @return The created game or `NULL` on failure.
 *
 * @pointer_lifetime The returned game is valid until the game is destroyed
 * via #rbtk_destroy_game(RBTK_GAME *).
 *
 * @debugging This function asserts that the functions in `funs` meet their
 * implementation requirements.
 * @errors
 * @signal{RBTK_ERROR_ILLEGAL_STATE,  If the game engine is not initialized.}
 * @signal{#RBTK_ERROR_OUT_OF_MEMORY, On memory allocation failure.}
 * @enderrors
 *
 * @see rbtk_add_game_state(RBTK_GAME *, RBTK_GAME_STATE *)
 * @see rbtk_start_game(RBTK_GAME *)
 */
RBTK_NO_DISCARD RBTK_GAME *
rbtk_create_game(rbtk_game_funs funs);

/*!
 * @brief Destroys a game.

 * @param[in] game The game to destroy.
 * @return `true` on success, `false` on failure.
 *
 * @errors
 * @signal{#RBTK_ERROR_ILLEGAL_STATE, If the game is currently running.}
 * @enderrors
 *
 * @see rbtk_stop_game(RBTK_GAME *)
 */
bool
rbtk_destroy_game(RBTK_GAME *game);

/*!
 * @brief Creates a game state.
 *
 * @param[in] funs The game state functions.
 * @return The created game state or `NULL` on failure.
 *
 * @pointer_lifetime The returned game state is valid until the game it
 * belongs to is destroyed.
 *
 * @debugging This function asserts that the functions in `funs` meet their
 * implementation requirements.
 * @errors
 * @signal{#RBTK_ERROR_OUT_OF_MEMORY, On memory allocation failure.}
 * @enderrors
 *
 * @see rbtk_add_game_state(RBTK_GAME *, RBTK_GAME_STATE *)
 */
RBTK_NO_DISCARD RBTK_GAME_STATE *
rbtk_create_game_state(rbtk_game_state_funs funs);

/*!
 * @brief Returns if a game has a game state.
 *
 * @param[in] game  The game to query.
 * @param[in] state The game state to check for.
 * @return `true` if the game has the specified state, `false` otherwise.
 *
 * @debugging This function asserts that `game` and `state` are not `NULL`.
 */
RBTK_NO_DISCARD bool
rbtk_game_has_game_state(const RBTK_GAME *game,
    const RBTK_GAME_STATE *state);

/*!
 * @brief Adds a game state to a game.
 *
 * @param[in] game  The game to add the state to.
 * @param[in] state The game state to add to the game.
 * @return `true` on success, `false` on failure.
 *
 * @debugging This function asserts that `game` and `state` are not `NULL`.
 * @errors
 * @signal{#RBTK_ERROR_ILLEGAL_STATE, If the game has already been started;
 *                                    If the game state already belongs to
 *                                    another game.}
 * @signal{#RBTK_ERROR_OUT_OF_MEMORY, If the game has already reached the
 *                                    maximum number of game states.}
 * @enderrors
 *
 * @see rbtk_enter_game_state(RBTK_GAME *, RBTK_GAME_STATE *, void *)
 */
bool
rbtk_add_game_state(RBTK_GAME *game, RBTK_GAME_STATE *state);

/*!
 * @brief Enters a game state.
 *
 * @param[in] game  The game switching states.
 * @param[in] state The game state to enter.
 * @param[in] args  Entrance arguments, may be `NULL`.
 * @return `true` on success, `false` on failure.
 *
 * @debugging This function asserts that `game` and `state` are not `NULL`.
 * @errors
 * @signal{#RBTK_ERROR_ILLEGAL_ARGUMENT, If the game state has not been
 *                                       added to the game.}
 * @enderrors
 */
bool
rbtk_enter_game_state(RBTK_GAME *game, RBTK_GAME_STATE *state, void *args);

/*!
 * @brief Exits the current game state of a game, if any.
 *
 * @param[in] game The game whose current state to exit.
 *
 * @debugging This function asserts that `game` is not `NULL`.
 */
void
rbtk_exit_game_state(RBTK_GAME *game);

/*! @} */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* RBTK_ENGINE_GAME_H_ */
