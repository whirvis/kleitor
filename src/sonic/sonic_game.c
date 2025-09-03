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
#include "sonic_game.h"

#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#include "../engine/engine.h"

#include "../runtime/runtime.h"

sonic_globals_type sonic_globals;
sonic_assets_type sonic_assets;

static void
create_game(RBTK_GAME *game)
{
    /* zero out memory just to be safe */
    memset(&sonic_globals, 0x00, sizeof(sonic_globals));
    memset(&sonic_assets, 0x00, sizeof(sonic_assets));

    RBTK_GAME_STATE *title =
        rbtk_create_game_state(sonic_title_state_funs);
    rbtk_add_game_state(game, title);
    sonic_globals.states.title = title;

    RBTK_GAME_STATE *load =
        rbtk_create_game_state(sonic_load_state_funs);
    rbtk_add_game_state(game, load);
    sonic_globals.states.load = load;

    RBTK_GAME_STATE *play =
        rbtk_create_game_state(sonic_play_state_funs);
    rbtk_add_game_state(game, play);
    sonic_globals.states.play = play;
}

static void
destroy_game(RBTK_UNUSED RBTK_GAME *game)
{
    /* nothing to do here yet */
}

static void
start_game(RBTK_GAME *game)
{
    RBTK_WINDOW *window = rbtk_get_primary_window();
    rbtk_set_window_title(window, "Sonic the Hedgehog CD");
    rbtk_set_window_size(window, SONIC_WINDOW_WIDTH, SONIC_WINDOW_HEIGHT);

    RBTK_ASSET *icon_asset = rbtk_get_asset("icon.png");
    if (icon_asset) {
        rbtk_set_window_icon(window, icon_asset);
    }

    RBTK_PROJECTION *proj = rbtk_create_greek_matrix(
        SONIC_SCREEN_WIDTH, SONIC_SCREEN_HEIGHT, 1000.0f);
    RBTK_GRAPHICS *scene = rbtk_create_scene(
        proj, SONIC_WINDOW_WIDTH, SONIC_WINDOW_HEIGHT);

    rbtk_bind_scene_to_window(window, scene);

    sonic_globals.window = window;
    sonic_globals.scene = scene;

    rbtk_enter_game_state(game, sonic_globals.states.title, NULL);
    rbtk_show_window(window);
}

static void
stop_game(RBTK_UNUSED RBTK_GAME *game)
{
    /* nothing to do here yet */
}

static void
pre_update(RBTK_UNUSED RBTK_GAME *game, RBTK_UNUSED long double delta)
{
    rbtk_update_io_device(rbtk_io_keyboard);
}

static void
post_update(RBTK_UNUSED RBTK_GAME *game, RBTK_UNUSED long double delta)
{
    if (rbtk_any_windows_should_close()) {
        rbtk_stop_game(game);
        return; /* do no more processing */
    }
}

static void
pre_render(RBTK_UNUSED RBTK_GAME *game)
{
    rbtk_clear_window_scene(sonic_globals.window);
}

static void
post_render(RBTK_UNUSED RBTK_GAME *game)
{
    rbtk_render_window_scene(sonic_globals.window);
}

const rbtk_game_funs sonic_game_funs = {
    .create = create_game,
    .destroy = destroy_game,
    .start = start_game,
    .stop = stop_game,
    .pre_update = pre_update,
    .post_update = post_update,
    .pre_render = pre_render,
    .post_render = post_render,
};

RBTK_NO_DISCARD int
rbtk_runtime_main(RBTK_UNUSED int argc, RBTK_UNUSED const char *argv[])
{
    if (!rbtk_engine_init()) {
        fprintf(stderr, "Failed to initialize game engine.\n");
        rbtk_abort_if_error();
    }

    RBTK_GAME *game = rbtk_create_game(sonic_game_funs);
    if (!game) {
        fprintf(stderr, "Failed to create game.\n");
        rbtk_abort_if_error();
    }

    if (!rbtk_start_game(game)) {
        fprintf(stderr, "Failed to start game.\n");
        rbtk_abort_if_error();
    }

    if (!rbtk_engine_terminate()) {
        fprintf(stderr, "Failed to terminate game engine.\n");
        rbtk_abort_if_error();
    }

    return EXIT_SUCCESS;
}
