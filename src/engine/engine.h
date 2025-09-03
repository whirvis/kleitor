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
#ifndef RBTK_ENGINE_ENGINE_H_
#define RBTK_ENGINE_ENGINE_H_

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/*!
 * @file
 * @brief The public API for the game engine.
 */

#include <stdbool.h>

#include "audio.h"
#include "game.h"
#include "graphics.h"
#include "input.h"

#include "../runtime/common.h"
#include "../runtime/error.h"

/*!
 * @defgroup engine Game Engine
 *
 * @brief The program's game engine.
 * @pre   Initialize the game engine.
 * @post  Terminate the game engine.
 *
 * The game engine is used to make video games under this program. As with
 * the rest of program, the engine is intended to be platform independent.
 * All modules intended for use purely with the game game engine shall also
 * be prefixed with `engine_`. If deemed necessary, a module from the game
 * engine shall become part of the program (or vice versa).
 *
 * @see rbtk_start_game(RBTK_GAME *)
 *
 * @{
 */

/*!
 * @brief Returns if the engine is initialized.
 *
 * @return `true` if the engine is initialized, `false` otherwise.
 */
bool
rbtk_engine_is_initialized(void);

/*!
 * @brief Initializes the game engine.
 *
 * @note If the engine is already initialized, this method is a no-op.
 *
 * @return `true` on success, `false` on failure.
 *
 * @errors
 * @signal{#RBTK_ERROR_PLATFORM, If a platform specific error occurred.}
 * @signal{#RBTK_ERROR_STARTUP,  If a module required by the engine failed
 *                               to initialize and did not signal an error
 *                               of its own.}
 * @enderrors
 */
bool
rbtk_engine_init(void);

/*!
 * @brief Terminates the game engine.
 *
 * @note If the engine is already terminated, this method is a no-op.
 *
 * @return `true` on success, `false` on failure.
 *
 * @errors
 * @signal{#RBTK_ERROR_PLATFORM, If a platform specific error occurred.}
 * @signal{#RBTK_ERROR_SHUTDOWN, If a module in use by the engine failed
 *                               to terminate and did not signal an error
 *                               of its own.}
 * @enderrors
 */
bool
rbtk_engine_terminate(void);

/*!
 * @brief Returns if a game is running.
 *
 * @param[in] game The game to query.
 * @return `true` if the game is running, `false` otherwise.
 *
 * @debugging This function asserts that `game` is not `NULL`.
 */
RBTK_NO_DISCARD bool
rbtk_game_is_running(const RBTK_GAME *game);

/*!
 * @brief Starts a game.
 * @pre Initialize the game engine.
 *
 * @param[in] game The game to start.
 * @return `true` on success, `false` on failure.
 *
 * @debugging This program asserts that `game` is not `NULL`.
 * @errors
 * @signal{#RBTK_ERROR_ILLEGAL_STATE, If the engine is not initialized;
 *                                    If a game is already running;
 *                                    If the game has already stopped.}
 * @enderrors
 *
 * @see rbtk_engine_init(void)
 */
bool
rbtk_start_game(RBTK_GAME *game);

/*!
 * @brief Stops a game.
 * @post Terminate the game engine.
 *
 * @note This function is a no-op if the game is not the one currently being
 * run by the engine.
 *
 * @param[in] game The game to start.
 *
 * @debugging This program asserts that `game` is not `NULL`.
 *
 * @see rbtk_engine_terminate(void)
 */
void
rbtk_stop_game(RBTK_GAME *game);

/*! @} */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* RBTK_ENGINE_ENGINE_H_ */
