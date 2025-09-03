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
#include "engine.h"
#include "./private/engine.h"
#include "./platform/engine.h"

#include <stdbool.h>

#include "audio.h"
#include "game.h"
#include "graphics.h"
#include "input.h"

#include "./private/audio.h"
#include "./private/game.h"
#include "./private/graphics.h"
#include "./private/input.h"

static RBTK_GAME *current_game;
static bool initialized;

bool
rbtk_engine_is_initialized(void)
{
    return initialized;
}

bool
rbtk_engine_init(void)
{
    if (initialized) {
        return true;
    }

    current_game = NULL;

    if (!plat_rbtk_engine_pre_init()) {
        rbtk_suggest_error(RBTK_ERROR_PLATFORM,
            "platform specific error during pre-initailization");
        return false;
    }
    if (!priv_rbtk_audio_init()) {
        rbtk_suggest_error(RBTK_ERROR_STARTUP,
            "audio module failed to initialize");
        return false;
    }
    if (!priv_rbtk_graphics_init()) {
        rbtk_suggest_error(RBTK_ERROR_STARTUP,
            "graphics module failed to initialize");
        return false;
    }
    if (!priv_rbtk_input_init()) {
        rbtk_suggest_error(RBTK_ERROR_STARTUP,
            "input module failed to initialize");
        return false;
    }
    if (!plat_rbtk_engine_post_init()) {
        rbtk_suggest_error(RBTK_ERROR_PLATFORM,
            "platform specific error during post-initailization");
        return false;
    }

    initialized = true;
    return true;
}

bool
rbtk_engine_terminate(void)
{
    if (!initialized) {
        return true;
    }

    if (rbtk_game_is_running(current_game)) {
        rbtk_stop_game(current_game);
    }

    current_game = NULL;

    if (!plat_rbtk_engine_pre_terminate()) {
        rbtk_suggest_error(RBTK_ERROR_PLATFORM,
            "platform specific error during pre-termination");
        return false;
    }
    if (!priv_rbtk_audio_terminate()) {
        rbtk_suggest_error(RBTK_ERROR_SHUTDOWN,
            "audio module failed to terminate");
        return false;
    }
    if (!priv_rbtk_audio_terminate()) {
        rbtk_suggest_error(RBTK_ERROR_SHUTDOWN,
            "graphics module failed to terminate");
        return false;
    }
    if (!priv_rbtk_input_terminate()) {
        rbtk_suggest_error(RBTK_ERROR_SHUTDOWN,
            "input module failed to terminate");
        return false;
    }
    if (!plat_rbtk_engine_post_terminate()) {
        rbtk_suggest_error(RBTK_ERROR_PLATFORM,
            "platform specific error during post-termination");
        return false;
    }

    initialized = false;
    return true;
}

RBTK_NO_DISCARD bool
rbtk_game_is_running(const RBTK_GAME *game)
{
    return game->running;
}

static void
start_game(void)
{
    assert(initialized);
    assert(current_game);

    RBTK_GAME *game = current_game;
    game->running = true;
    if (game->funs.start) {
        game->funs.start(game);
    }
    game->last_update = rbtk_time(RBTK_MILLIS);
}

static void
stop_game(void)
{
    assert(initialized);
    assert(current_game);

    RBTK_GAME *game = current_game;

    rbtk_exit_game_state(game);
    for (size_t i = 0; i < game->state_count; i++) {
        RBTK_GAME_STATE *state = game->states[i];
        if (state->funs.deinit) {
            state->funs.deinit(game, state);
        }
    }

    if (game->funs.stop) {
        game->funs.stop(game);
    }
}

static void
update_engine(void)
{
    assert(initialized);
    assert(current_game);

    RBTK_GAME *game = current_game;

    long double current_time = rbtk_time(RBTK_MILLIS);
    long double delta = current_time - game->last_update;
    game->last_update = current_time;

    plat_rbtk_engine_pre_update();
    if (game->funs.pre_update) {
        game->funs.pre_update(game, delta);
    }

    RBTK_GAME_STATE *state = game->current_state;
    if (state && state->funs.update) {
        state->funs.update(game, state, delta);
    }

    if (game->funs.post_update) {
        game->funs.post_update(game, delta);
    }
    plat_rbtk_engine_post_update();
}

static void
render_engine(void)
{
    assert(initialized);
    assert(current_game);

    RBTK_GAME *game = current_game;

    plat_rbtk_engine_pre_render();
    if (game->funs.pre_render) {
        game->funs.pre_render(game);
    }

    RBTK_GAME_STATE *state = game->current_state;
    if (state && state->funs.render) {
        state->funs.render(game, state);
    }

    if (game->funs.post_render) {
        game->funs.post_render(game);
    }
    plat_rbtk_engine_post_render();
}

bool
rbtk_start_game(RBTK_GAME *game)
{
    assert(game);

    if (current_game) {
        rbtk_signal_error(RBTK_ERROR_ILLEGAL_STATE,
            "already running a game");
        return false;
    }
    else if (game->running) {
        rbtk_signal_error(RBTK_ERROR_ILLEGAL_STATE,
            "game is already running");
        return false;
    }
    else if (game->stopped) {
        rbtk_signal_error(RBTK_ERROR_ILLEGAL_STATE,
            "game cannot be started after being stopepd");
        return false;
    }

    current_game = game;

    start_game();
    while (game->running) {
        update_engine();
        render_engine();
    }
    stop_game();

    return true;
}

void
rbtk_stop_game(RBTK_GAME *game)
{
    assert(game);
    if (current_game == game) {
        game->running = false;
        game->stopped = true;
    }
}
