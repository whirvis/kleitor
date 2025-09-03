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

/*
 * These values were determined by painful trial and error, because for some
 * reason Sonic's bust sprite is larger than actually shown in game. For the
 * love of Christ, do not change these without sufficient testing.
 */
#define SONIC_BUST_X_OFFSET                92.0f
#define SONIC_BUST_Y_OFFSET                20.0f
#define SONIC_BUST_X_SCALE                  0.75f
#define NUM_SONIC_BUST_FRAMES               5
#define SONIC_BUST_APPEAR_DURATION          0.25f
#define SONIC_BUST_RAISED_EYEBROW_X_OFFSET  4
#define SONIC_BUST_RAISED_EYEBROW_Y_OFFSET  24
#define NUM_FINGER_WAG_FRAMES               4
#define FINGER_WAG_DURATION                 0.15f

static struct {
    float x; float y;
} const FINGER_WAG_OFFSETS[NUM_FINGER_WAG_FRAMES] = {
    { .x = 59, .y = 49 },
    { .x = 59, .y = 43 },
    { .x = 58, .y = 43 },
    { .x = 49, .y = 43 },
};

static struct {
    RBTK_SPRITE_ANIME *anime;
    RBTK_SPRITE_ANIME *finger_wag;
    RBTK_SPRITE *sprite;
    RBTK_SPRITE *raised_eyebrow;
    unsigned int wag_finger_count;
    bool wagged_finger;
    RBTK_PROJECTION *proj;
    RBTK_GRAPHICS *scene;
} sonic_bust;

typedef enum sonic_bust_render_mode {
    SONIC_BUST_SUSPENSE,
    SONIC_BUST_NATURAL
} sonic_bust_render_mode;

#define INTRO_STATE_SUSPENSE 0
#define INTRO_STATE_REVEAL   1
#define INTRO_STATE_DONE     2

#define SUSPENSE_WAIT_MS 1000
#define REVEAL_WAIT_MS   SUSPENSE_WAIT_MS + 25
#define FLASH_FADE_SPEED 0.0005f

static bool intro_theme_easter_egg;

static struct {
    int state;
    long double suspense_timer;
    struct {
        RBTK_SOUND *intro;
        RBTK_SOUND *loop;
        bool started_intro;
        bool started_loop;
    } theme;
    struct {
        RBTK_SPRITE *sprite;
        float alpha;
    } flash;
    RBTK_SPRITE *sky;
} intro_sequence;

#define OUTRO_FADE_SPEED  0.001f
#define OUTRO_FADE_FINISH 1.25f

static struct {
    RBTK_SPRITE *black;
    bool in_progress;
    float fade_progress;
    bool finished;
} outro_sequence;

#define PRESS_PROMPT_INITIAL_DISPLAY_TIME_MS 500

static struct {
    RBTK_SPRITE *banner;
    RBTK_SPRITE *bg;
    RBTK_SPRITE *c_sega_1993;
    RBTK_SPRITE *flash;
    RBTK_SPRITE *medal;
    RBTK_SPRITE *tm;
    struct {
        RBTK_SPRITE *sprite;
        long double timer;
        long double display_time;
        bool show;
    } press_prompt;
} foreground;

/*
 * These values were determined by trial and error using the debugger
 * and moving the camera around in 3D space. Please do not change them
 * without sufficient testing first.
 */
#define LITTLE_PLANET_X_OFFSET     20
#define LITTLE_PLANET_Y_OFFSET    -25
#define LITTLE_PLANET_FLOAT_SPEED   0.05f

#define NUM_CLOUDS                   2
#define CLOUD_SPEED                  0.05f
#define CLOUD_X_OFFSET               0.0f
#define CLOUD_Y_OFFSET              90.0f
#define CLOUD_FINAL_Z_OFFSET       128.0f
#define CLOUD_X_ROTATION             1.67f
#define cloud_initial_z_offset(_i) ((256.0f + (64.0f * (_i))) * -1.0f)

#define NUM_LAKES                  4
#define LAKE_SPEED                 0.1f
#define LAKE_INITIAL_X_OFFSET   -320.0f
#define LAKE_FINAL_X_OFFSET      320.0f
#define LAKE_Y_OFFSET            160.0f
#define LAKE_Z_OFFSET              0.0f
#define LAKE_X_SCALE               1.025f
#define LAKE_X_ROTATION            0.8f
#define setup_lake_x_offset(_i) ((256.0f * (_i)) - 64.0f)

static struct {
    RBTK_SPRITE *sprite;
    RBTK_SPRITE *little_planet;
    struct {
        RBTK_SPRITE *sprite;
        float z_offsets[NUM_CLOUDS];
    } clouds;
    struct {
        RBTK_SPRITE *sprite;
        float x_offsets[NUM_LAKES];
    } lakes;
    RBTK_CAMERA *camera;
    RBTK_PROJECTION *proj;
    RBTK_GRAPHICS *scene;
} sky;

static void
init_sonic_bust(void)
{
    RBTK_ZERO_MEMORY(&sonic_bust);

    sonic_bust.anime = sonic_assets.sprites.title.sonic_bust_appear;
    sonic_bust.finger_wag = sonic_assets.sprites.title.sonic_finger_wag;
    sonic_bust.sprite = sonic_assets.sprites.title.sonic_bust;
    sonic_bust.raised_eyebrow = sonic_assets.sprites.title.sonic_bust_raised_eyebrow;

    for (int i = 0; i < NUM_FINGER_WAG_FRAMES; i++) {
        RBTK_SPRITE *sprite = rbtk_get_sprite_in_anime(sonic_bust.finger_wag, i);
        rbtk_set_sprite_offset(sprite, FINGER_WAG_OFFSETS[i].x, FINGER_WAG_OFFSETS[i].y, 0.0f);
    }

    rbtk_set_sprite_offset(sonic_bust.raised_eyebrow, SONIC_BUST_RAISED_EYEBROW_X_OFFSET,
        SONIC_BUST_RAISED_EYEBROW_Y_OFFSET, 0.0f);

    /*
     * Do not loop the appear animation. We want the state to know when it is
     * done so it can transition to the next state of the intro sequence. The
     * same goes for the finger wagging animation, so it knows when it should
     * start and stop wagging the finger.
     */
    rbtk_loop_sprite_anime(sonic_bust.anime, false, false);
    rbtk_loop_sprite_anime(sonic_bust.finger_wag, false, false);

    /*
     * Determine the largest width and height among the frames in the appear
     * animation. We'll use these values for creating the scene to render to.
     */
    unsigned int bust_width = 0, bust_height = 0;
    for (int i = 0; i < NUM_SONIC_BUST_FRAMES; i++) {
        unsigned int width, height;
        RBTK_SPRITE *sprite = rbtk_get_sprite_in_anime(sonic_bust.anime, i);
        rbtk_get_sprite_size(sprite, &width, &height);
        if (width > bust_width) {
            bust_width = width;
        }
        if (height > bust_height) {
            bust_height = height;
        }
    }

    sonic_bust.wag_finger_count = 0;

    sonic_bust.proj = rbtk_create_greek_matrix(
        (float) bust_width, (float) bust_height, 100.0f);
    sonic_bust.scene = rbtk_create_scene(sonic_bust.proj,
        bust_width, bust_height);

    /*
     * Scale the whole scene rather than the sprites themself. This makes it
     * much easier to render to as we don't need to update each sprite of the
     * bust individually.
     */
    RBTK_SPRITE *scene_sprite = rbtk_get_scene_sprite(sonic_bust.scene);
    rbtk_scale_sprite(scene_sprite, SONIC_BUST_X_SCALE, 1.0f, 1.0f);
    rbtk_set_sprite_offset(scene_sprite, SONIC_BUST_X_OFFSET, SONIC_BUST_Y_OFFSET, 1.0f);
}

static void
deinit_sonic_bust(void)
{
    rbtk_destroy_scene(sonic_bust.scene);
    rbtk_destroy_projection(sonic_bust.proj);
    RBTK_ZERO_MEMORY(&sonic_bust);
}

static void
update_sonic_bust(long double delta_ms)
{
    if (intro_sequence.flash.alpha <= 0.0f && !sonic_bust.wagged_finger) {
        sonic_bust.wag_finger_count += 2;
        sonic_bust.wagged_finger = true;
    }

    if (sonic_bust.wag_finger_count > 0) {
        bool playing = !rbtk_sprite_anime_is_finished(sonic_bust.finger_wag);
        bool backwards = rbtk_sprite_anime_is_playing_backwards(sonic_bust.finger_wag);

        if (!playing && !backwards) {
            rbtk_play_sprite_anime_backwards(sonic_bust.finger_wag, true);
            rbtk_restart_sprite_anime(sonic_bust.finger_wag);
            playing = true;
        }

        if (playing) {
            rbtk_update_sprite_anime(sonic_bust.finger_wag, delta_ms, RBTK_MILLIS);
        }

        if (!playing && backwards) {
            rbtk_play_sprite_anime_backwards(sonic_bust.finger_wag, false);
            rbtk_restart_sprite_anime(sonic_bust.finger_wag);
            sonic_bust.wag_finger_count -= 1;
        }
    }
}

static void
render_sonic_bust(sonic_bust_render_mode mode)
{
    rbtk_clear_scene(sonic_bust.scene);
    if (mode == SONIC_BUST_SUSPENSE) {
        rbtk_draw_sprite_anime_at_offset(sonic_bust.scene, sonic_bust.anime);
    } else if (mode == SONIC_BUST_NATURAL) {
        rbtk_draw_sprite_at_offset(sonic_bust.scene, sonic_bust.sprite);
        if (sonic_bust.wag_finger_count > 0) {
            rbtk_draw_sprite_at_offset(sonic_bust.scene, sonic_bust.raised_eyebrow);
        }
        rbtk_draw_sprite_anime_at_offset(sonic_bust.scene, sonic_bust.finger_wag);
    } else {
        assert(0); /* we forgot one! */
    }
}

static void
init_intro(void)
{
    RBTK_ZERO_MEMORY(&intro_sequence);

    intro_sequence.state = INTRO_STATE_SUSPENSE;
    intro_sequence.suspense_timer = 0.0l;
    if (intro_theme_easter_egg) {
        intro_sequence.theme.intro = sonic_assets.ost.title.title_theme_ym2612_intro;
        intro_sequence.theme.loop = sonic_assets.ost.title.title_theme_ym2612_loop;
    } else {
        intro_sequence.theme.intro = sonic_assets.ost.title.title_theme_intro;
        intro_sequence.theme.loop = sonic_assets.ost.title.title_theme_loop;
    }
    intro_sequence.theme.started_intro = false;
    intro_sequence.theme.started_loop = false;
    intro_sequence.flash.sprite = sonic_assets.sprites.title.flash;
    intro_sequence.flash.alpha = 1.0f;
    intro_sequence.sky = sonic_assets.sprites.title.sky;
}

static void
deinit_intro(void)
{
    /* TODO: make the theme fade-out instead */
    rbtk_stop_sound(intro_sequence.theme.intro);
    rbtk_stop_sound(intro_sequence.theme.loop);
    RBTK_ZERO_MEMORY(&intro_sequence);
}

static void
update_intro(long double delta_ms)
{
    /*
     * The first part of the title state has sonic with has backed turned to
     * give an aura of suspense. After the suspense time is up, we immeidately
     * start the title theme. There is also a reveal timer, which is just the
     * suspense wait with a small delay. This is better synchronizes the title
     * theme with the overall intro sequence.
     */
    if (intro_sequence.state == INTRO_STATE_SUSPENSE) {
        intro_sequence.suspense_timer += delta_ms;

        if (intro_sequence.suspense_timer >= SUSPENSE_WAIT_MS
                && !intro_sequence.theme.started_intro) {
            /* also ensure it starts from the beginning */
            rbtk_stop_sound(intro_sequence.theme.intro);
            rbtk_play_sound(intro_sequence.theme.intro);
            intro_sequence.theme.started_intro = true;
        }

        if (intro_sequence.suspense_timer >= REVEAL_WAIT_MS) {
            rbtk_restart_sprite_anime(sonic_bust.anime);
            rbtk_restart_sprite_anime(sonic_bust.finger_wag);
            intro_sequence.state = INTRO_STATE_REVEAL;
        }
    }

    /*
     * In this state, we're revealing Sonic as he turns around with his bust
     * animation. As soon as it's finished, we mark the intro sequence state
     * as done.
     */
    if (intro_sequence.state == INTRO_STATE_REVEAL) {
        rbtk_update_sprite_anime(sonic_bust.anime, delta_ms, RBTK_MILLIS);
        if (rbtk_sprite_anime_is_finished(sonic_bust.anime)) {
            intro_sequence.flash.alpha = 1.0f;
            intro_sequence.state = INTRO_STATE_DONE;
        }
    }

    /*
     * Once the intro sequence is done, we fade out the flash of white that
     * appears in front of the user. This flash is there to transition from
     * the blank blue screen to the vibrant foreground and sky shown to the
     * user after the intro sequence.
     */
    if (intro_sequence.state == INTRO_STATE_DONE) {
        intro_sequence.flash.alpha -= FLASH_FADE_SPEED * (float) delta_ms;
        rbtk_set_sprite_alpha(intro_sequence.flash.sprite,
            intro_sequence.flash.alpha);
    }

    /*
     * After playing the intro of the title theme, we need to wait for it to
     * finish playing. As soon as it's finished, we start the loop. This way,
     * the intro is only heard once and the loop is seamless.
     */
    if (!intro_sequence.theme.started_loop) {
        rbtk_sound_state intro_state =
            rbtk_get_sound_state(intro_sequence.theme.intro);
        if (intro_sequence.theme.started_intro
            && intro_state != RBTK_SOUND_STATE_PLAYING) {
            rbtk_stop_sound(intro_sequence.theme.loop);
            rbtk_loop_sound(intro_sequence.theme.loop, true);
            rbtk_play_sound(intro_sequence.theme.loop);
            intro_sequence.theme.started_loop = true;
        }
    }
}

static void
render_intro(RBTK_GRAPHICS *scene)
{
    if (intro_sequence.state == INTRO_STATE_DONE) {
        return; /* intro is finished, nothing to render */
    }
    render_sonic_bust(SONIC_BUST_SUSPENSE);
    rbtk_draw_sprite_at_offset(scene, intro_sequence.sky);
    rbtk_draw_scene_at_offset(scene, sonic_bust.scene);
}

static void
init_outro(void)
{
    RBTK_ZERO_MEMORY(&outro_sequence);

    outro_sequence.black = sonic_assets.sprites.title.black;
    outro_sequence.in_progress = false;
    outro_sequence.fade_progress = 0.0f;
    outro_sequence.finished = false;
}

static void
deinit_outro(void)
{
    RBTK_ZERO_MEMORY(&outro_sequence);
}

static void
update_outro(long double delta_ms)
{
    if (outro_sequence.in_progress) {
        outro_sequence.fade_progress += OUTRO_FADE_SPEED * (float) delta_ms;
        rbtk_set_sprite_alpha(outro_sequence.black, outro_sequence.fade_progress);

        float volume = 1.0f - outro_sequence.fade_progress;
        rbtk_set_sound_volume(intro_sequence.theme.loop, volume);
        rbtk_set_sound_volume(intro_sequence.theme.intro, volume);
    }

    if (outro_sequence.fade_progress >= OUTRO_FADE_FINISH) {
        outro_sequence.finished = true;
    }
}

static void
render_outro(RBTK_GRAPHICS *scene)
{
    if (outro_sequence.in_progress) {
        rbtk_draw_sprite_at_offset(scene, outro_sequence.black);
    }
}

static void
init_foreground(void)
{
    foreground.banner = sonic_assets.sprites.title.banner;
    foreground.bg = sonic_assets.sprites.title.bg;
    foreground.c_sega_1993 = sonic_assets.sprites.title.c_sega_1993;
    foreground.flash = sonic_assets.sprites.title.flash;
    foreground.medal = sonic_assets.sprites.title.medal;
    foreground.tm = sonic_assets.sprites.title.tm;

    foreground.press_prompt.sprite = sonic_assets.sprites.title.press_enter;
    foreground.press_prompt.timer = 0.0f;
    foreground.press_prompt.display_time = PRESS_PROMPT_INITIAL_DISPLAY_TIME_MS;
    foreground.press_prompt.show = true;

    /* center the medal to the middle of the screen */
    unsigned int medal_width, medal_height;
    rbtk_get_sprite_size(foreground.medal, &medal_width, &medal_height);
    rbtk_set_sprite_offset(foreground.medal,
        (SONIC_SCREEN_WIDTH - medal_width) / 2.0f,
        (SONIC_SCREEN_HEIGHT - medal_height) / 2.0f, 1.0f);

    float medal_y;  /* get the medal's Y-axis for other sprites */
    rbtk_get_sprite_offset(foreground.medal, NULL, &medal_y, NULL);

    /* center banner and place at bottom of medal */
    unsigned int banner_width, banner_height;
    rbtk_get_sprite_size(foreground.banner,
        &banner_width, &banner_height);
    rbtk_set_sprite_offset(foreground.banner,
        (SONIC_SCREEN_WIDTH - banner_width) / 2.0f,
        (medal_y + medal_height - banner_height) + 19, 1.0f);

    /* place trademark by top right of banner */
    unsigned int trademark_width;
    float banner_x, banner_y;
    rbtk_get_sprite_size(foreground.tm, &trademark_width, NULL);
    rbtk_get_sprite_offset(foreground.banner, &banner_x, &banner_y, NULL);
    rbtk_set_sprite_offset(foreground.tm,
        (banner_x + banner_width) - trademark_width, banner_y, 1.0f);

    /* center "Press Start" and place below medal */
    unsigned int press_prompt_width, press_prompt_height;
    rbtk_get_sprite_size(foreground.press_prompt.sprite,
        &press_prompt_width, &press_prompt_height);
    rbtk_set_sprite_offset(foreground.press_prompt.sprite,
        (SONIC_SCREEN_WIDTH - press_prompt_width) / 2.0f,
        medal_y + medal_height + 6, 1.0f);

    /* center copyright to the middle of the screen */
    unsigned int c_sega_width, c_sega_height;
    rbtk_get_sprite_size(foreground.c_sega_1993,
        &c_sega_width, &c_sega_height);
    rbtk_set_sprite_offset(foreground.c_sega_1993,
        (SONIC_SCREEN_WIDTH - c_sega_width) / 2.0f,
        (SONIC_SCREEN_HEIGHT - c_sega_height) - 5.0f, 1.0f);
}

static void
deinit_foreground(void)
{
    RBTK_ZERO_MEMORY(&foreground);
}

static void
update_foreground(long double delta_ms)
{
    foreground.press_prompt.timer += delta_ms;
    long double display_time = foreground.press_prompt.display_time;
    if (display_time <= 0) {
        foreground.press_prompt.show = false;
    } else {
        if (foreground.press_prompt.timer >= display_time) {
            foreground.press_prompt.timer -= display_time;
            foreground.press_prompt.show ^= true;
        }
    }
}

static void
render_foreground(RBTK_GRAPHICS *scene)
{
    render_sonic_bust(SONIC_BUST_NATURAL);

    rbtk_draw_sprite_at_offset(scene, foreground.bg);
    rbtk_draw_sprite_at_offset(scene, foreground.medal);
    rbtk_draw_scene_at_offset(scene, sonic_bust.scene);
    rbtk_draw_sprite_at_offset(scene, foreground.banner);
    rbtk_draw_sprite_at_offset(scene, foreground.tm);
    rbtk_draw_sprite_at_offset(scene, foreground.c_sega_1993);
    if (foreground.press_prompt.show) {
        rbtk_draw_sprite_at_offset(scene, foreground.press_prompt.sprite);
    }
    rbtk_draw_sprite_at_offset(scene, foreground.flash);
}

static void
init_sky(void)
{
    RBTK_ZERO_MEMORY(&sky);

    sky.sprite = sonic_assets.sprites.title.sky;

    sky.little_planet = sonic_assets.sprites.title.little_planet;
    rbtk_set_sprite_offset(sky.little_planet,
        (SONIC_SCREEN_WIDTH - 96) + LITTLE_PLANET_X_OFFSET,
        LITTLE_PLANET_Y_OFFSET, 0.0f);

    sky.clouds.sprite = sonic_assets.sprites.title.clouds;
    rbtk_set_sprite_alpha(sky.clouds.sprite, 0.90f);
    rbtk_rotate_sprite_to(sky.clouds.sprite, CLOUD_X_ROTATION, 0, 0);
    for (size_t i = 0; i < NUM_CLOUDS; i++) {
        sky.clouds.z_offsets[i] = cloud_initial_z_offset(i);
    }

    sky.lakes.sprite = sonic_assets.sprites.title.lake;
    rbtk_scale_sprite(sky.lakes.sprite, LAKE_X_SCALE, 1, 1);
    rbtk_rotate_sprite_to(sky.lakes.sprite, LAKE_X_ROTATION, 0, 0);
    for (size_t i = 0; i < NUM_LAKES; i++) {
        sky.lakes.x_offsets[i] = setup_lake_x_offset(i);
    }

    sky.proj = rbtk_create_persp_projection(90.0f,
        SONIC_SCREEN_WIDTH, SONIC_SCREEN_HEIGHT, 0.1f, 1000.0f);
    sky.scene = rbtk_create_scene(sky.proj,
        SONIC_SCREEN_WIDTH, SONIC_SCREEN_HEIGHT);
    sky.camera = rbtk_get_scene_camera(sky.scene);

    rbtk_center_camera_greekways(sky.scene);
}

static void
deinit_sky(void)
{
    rbtk_destroy_projection(sky.proj);
    rbtk_destroy_scene(sky.scene);
    RBTK_ZERO_MEMORY(&sky);
}

static void
update_sky(long double delta_ms)
{
    for (size_t i = 0; i < NUM_CLOUDS; i++) {
        float z = sky.clouds.z_offsets[i]
            + (CLOUD_SPEED * (float) delta_ms);

        /*
         * Some updates will cause the Z offset of the sprite
         * to go far past the intended final Z offset. By using
         * a while loop, we can simulate the sprite moving past
         * multiple times until it reaches where it should be.
         */
        if (z >= CLOUD_FINAL_Z_OFFSET) {
            float initial_z = cloud_initial_z_offset(i);
            while (z >= CLOUD_FINAL_Z_OFFSET) {
                float diff = z - CLOUD_FINAL_Z_OFFSET;
                z = initial_z + diff;
            }
        }

        sky.clouds.z_offsets[i] = z;
    }

    for (size_t i = 0; i < NUM_LAKES; i++) {
        float x = sky.lakes.x_offsets[i] 
            + (LAKE_SPEED * (float) delta_ms);

        /*
         * Some updates will cause the X offset of the sprite
         * to go far past the intended final X offset. By using
         * a while loop, we can simulate the sprite moving past
         * multiple times until it reaches where it should be.
         */
        while (x >= LAKE_FINAL_X_OFFSET) {
            float diff = x - LAKE_FINAL_X_OFFSET;
            x = LAKE_INITIAL_X_OFFSET + diff;
        }

        sky.lakes.x_offsets[i] = x;
    }
}

static void
render_sky(RBTK_GRAPHICS *scene)
{
    rbtk_clear_scene(sky.scene);

    rbtk_draw_sprite_at_offset(sky.scene, sky.sprite);

    double time = (double) rbtk_time(RBTK_SECS);
    float little_planet_float_y = (float) (sin(time) * 5.0);
    rbtk_draw_sprite(sky.scene, sky.little_planet,
        0, little_planet_float_y, 0);

    for (int i = 0; i < NUM_CLOUDS; i++) {
        rbtk_draw_sprite(sky.scene, sky.clouds.sprite,
            CLOUD_X_OFFSET, CLOUD_Y_OFFSET, sky.clouds.z_offsets[i]);
    }
    for (int i = 0; i < NUM_LAKES; i++) {
        rbtk_draw_sprite(sky.scene, sky.lakes.sprite,
            sky.lakes.x_offsets[i], LAKE_Y_OFFSET, LAKE_Z_OFFSET);
    }

    rbtk_draw_scene_at_offset(scene, sky.scene);
}

static void
init_state(RBTK_UNUSED RBTK_GAME *game, RBTK_UNUSED RBTK_GAME_STATE *state)
{
    srand((unsigned int) rbtk_time(RBTK_NANOS));
    intro_theme_easter_egg = (rand() % 10 == 0);

    sonic_buffer_sfx(menu, select);

    if (intro_theme_easter_egg) {
        sonic_buffer_ost(title, title_theme_ym2612_intro);
        sonic_buffer_ost(title, title_theme_ym2612_loop);
    } else {
        sonic_buffer_ost(title, title_theme_intro);
        sonic_buffer_ost(title, title_theme_loop);
    }

    sonic_load_sprite_anime(title, sonic_bust_appear,
        NUM_SONIC_BUST_FRAMES, SONIC_BUST_APPEAR_DURATION, RBTK_SECS);
    sonic_load_sprite_anime(title, sonic_finger_wag,
        NUM_FINGER_WAG_FRAMES, FINGER_WAG_DURATION, RBTK_SECS);

    sonic_load_sprite(title, banner);
    sonic_load_sprite(title, bg);
    sonic_load_sprite(title, black);
    sonic_load_sprite(title, c_sega_1993);
    sonic_load_sprite(title, clouds);
    sonic_load_sprite(title, flash);
    sonic_load_sprite(title, lake);
    sonic_load_sprite(title, little_planet);
    sonic_load_sprite(title, medal);
    sonic_load_sprite(title, press_enter);
    sonic_load_sprite(title, press_start);
    sonic_load_sprite(title, sky);
    sonic_load_sprite(title, sonic_bust);
    sonic_load_sprite(title, sonic_bust_raised_eyebrow);
    sonic_load_sprite(title, tm);
}

static void
deinit_state(RBTK_UNUSED RBTK_GAME *game,
        RBTK_UNUSED RBTK_GAME_STATE *state)
{
    sonic_close_sfx(menu, select);

    if (intro_theme_easter_egg) {
        sonic_close_ost(title, title_theme_ym2612_intro);
        sonic_close_ost(title, title_theme_ym2612_loop);
    } else {
        sonic_close_ost(title, title_theme_intro);
        sonic_close_ost(title, title_theme_loop);
    }

    sonic_unload_sprite_anime(title, sonic_bust_appear);
    sonic_unload_sprite_anime(title, sonic_finger_wag);

    sonic_unload_sprite(title, banner);
    sonic_unload_sprite(title, bg);
    sonic_unload_sprite(title, black);
    sonic_unload_sprite(title, c_sega_1993);
    sonic_unload_sprite(title, clouds);
    sonic_unload_sprite(title, flash);
    sonic_unload_sprite(title, lake);
    sonic_unload_sprite(title, little_planet);
    sonic_unload_sprite(title, medal);
    sonic_unload_sprite(title, press_enter);
    sonic_unload_sprite(title, press_start);
    sonic_unload_sprite(title, sky);
    sonic_unload_sprite(title, sonic_bust);
    sonic_unload_sprite(title, sonic_bust_raised_eyebrow);
    sonic_unload_sprite(title, tm);
}

static void
enter_state(RBTK_UNUSED RBTK_GAME *game,
        RBTK_UNUSED RBTK_GAME_STATE *state, RBTK_UNUSED void *args) {
    init_sonic_bust();
    init_intro();
    init_outro();
    init_sky();
    init_foreground();
}

static void
exit_state(RBTK_UNUSED RBTK_GAME *game,
        RBTK_UNUSED RBTK_GAME_STATE *state) {
    deinit_sonic_bust();
    deinit_intro();
    deinit_outro();
    deinit_sky();
    deinit_foreground();
}

static void
update_state(RBTK_UNUSED RBTK_GAME *game,
        RBTK_UNUSED RBTK_GAME_STATE *state, long double delta_ms)
{
    update_sonic_bust(delta_ms);
    update_intro(delta_ms);
    update_outro(delta_ms);
    update_foreground(delta_ms);
    update_sky(delta_ms);

    if (rbtk_io_keyboard_state->enter->just_pressed
            && !outro_sequence.in_progress) {
        outro_sequence.in_progress = true;
        foreground.press_prompt.display_time = 5;
        rbtk_play_sound(sonic_assets.sfx.menu.select);
    }

    /* TODO: enter play state when applicable */
    if (outro_sequence.finished) {
        rbtk_exit_game_state(game);
        rbtk_stop_game(game);
    }
}

static void
render_state(RBTK_UNUSED RBTK_GAME *game,
        RBTK_UNUSED RBTK_GAME_STATE *state)
{
    if (intro_sequence.state != INTRO_STATE_DONE) {
        render_intro(sonic_globals.scene);
    } else {
        render_sky(sonic_globals.scene);
        render_foreground(sonic_globals.scene);
        render_outro(sonic_globals.scene);
    }
}

const rbtk_game_state_funs sonic_title_state_funs = {
    .init = init_state,
    .deinit = deinit_state,
    .enter = enter_state,
    .exit = exit_state,
    .update = update_state,
    .render = render_state,
};
