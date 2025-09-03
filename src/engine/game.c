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
#include "game.h"
#include "./private/game.h"
#include "./platform/game.h"

#include "engine.h"

#include <assert.h>
#include <stdlib.h>

RBTK_NO_DISCARD RBTK_GAME *
rbtk_create_game(rbtk_game_funs funs)
{
    if (!rbtk_engine_is_initialized()) {
        rbtk_signal_error(RBTK_ERROR_ILLEGAL_STATE,
            "engine must be initialized");
        return NULL;
    }

    RBTK_GAME *game = NULL;
    RBTK_MALLOC_OR_RETURN(&game, NULL,
        "could not allocate memory for game");

    game->funs = funs;
    game->state_count = 0;
    game->current_state = NULL;
    game->last_update = 0;
    game->running = false;
    game->stopped = false;

    if (funs.create) {
        funs.create(game);
    }

    return game;
}

bool
rbtk_destroy_game(RBTK_GAME *game)
{
    if (!game) {
        return true;
    }

    if (game->running) {
        rbtk_signal_error(RBTK_ERROR_ILLEGAL_STATE,
            "cannot destroy a game while it is running");
        return false;
    }

    for (size_t i = 0; i < game->state_count; i++) {
        RBTK_GAME_STATE *state = game->states[i];
        free(state); /* no longer needed */
    }

    return true;
}

RBTK_NO_DISCARD RBTK_GAME_STATE *
rbtk_create_game_state(rbtk_game_state_funs funs)
{
    RBTK_GAME_STATE *state = NULL;
    RBTK_MALLOC_OR_RETURN(&state, NULL,
        "could not allocate memory for game state");

    state->owner = NULL;
    state->funs = funs;

    return state;
}

RBTK_NO_DISCARD bool
rbtk_game_has_game_state(const RBTK_GAME *game,
    const RBTK_GAME_STATE *state)
{
    assert(game && state);
    return state->owner == game;
}

bool
rbtk_add_game_state(RBTK_GAME *game, RBTK_GAME_STATE *state)
{
    assert(game && state);
    if (state->owner == game) {
        return true; /* already added, don't fuss */
    }

    if (game->running || game->stopped) {
        rbtk_signal_error(RBTK_ERROR_ILLEGAL_STATE,
            "game state must be added before game is started");
        return false;
    }

    if (state->owner && state->owner != game) {
        rbtk_signal_error(RBTK_ERROR_ILLEGAL_STATE,
            "game state already belongs to another game");
    }

    if (game->state_count >= RBTK_MAX_GAME_STATES) {
        rbtk_signal_error(RBTK_ERROR_OUT_OF_MEMORY,
            "max number of game states reached");
        return false;
    }

    state->owner = game;
    if(state->funs.init) {
        state->funs.init(game, state);
    }
    game->states[game->state_count] = state;
    game->state_count += 1;

    return true;
}

bool
rbtk_enter_game_state(RBTK_GAME *game, RBTK_GAME_STATE *state, void *args)
{
    assert(game && state);

    if (!rbtk_game_has_game_state(game, state)) {
        rbtk_signal_error(RBTK_ERROR_ILLEGAL_ARGUMENT,
            "game state not a part of game");
        return false;
    }

    rbtk_exit_game_state(game);
    if (state->funs.enter) {
        state->funs.enter(game, state, args);
    }
    game->current_state = state;

    return true;
}

void
rbtk_exit_game_state(RBTK_GAME *game)
{
    assert(game);

    RBTK_GAME_STATE *state = game->current_state;
    if (!state) {
        return;
    }

    if (state->funs.exit) {
        state->funs.exit(game, state);
    }
    game->current_state = NULL;
}
