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
#ifndef SONIC_GAME_H_
#define SONIC_GAME_H_

/*!
 * @file
 * @brief The basis for the Sonic the Hedgehog game.
 */

#include <string.h>

#include "../engine/engine.h"

#define SONIC_WINDOW_WIDTH  1024
#define SONIC_WINDOW_HEIGHT 768

#define SONIC_SCREEN_WIDTH  256
#define SONIC_SCREEN_HEIGHT 224

#define sonic_load_sprite(_object, _name)                             \
    do {                                                              \
        if (!sonic_assets.sprites._object._name) {                    \
	        const char *path = "sprites/" #_object "/" #_name ".png"; \
	        RBTK_ASSET *asset = rbtk_require_asset(path);             \
                                                                      \
	        RBTK_SPRITE *sprite = rbtk_load_sprite(asset);            \
                if (!sprite) {                                        \
                    sonic_assets.sprites._object._name = NULL;        \
                    break; /* error loading sprite*/                  \
                }                                                     \
                                                                      \
	        sonic_assets.sprites._object._name = sprite;              \
        }                                                             \
    } while (0)

#define sonic_unload_sprite(_object, _name)                         \
    do {                                                            \
        if (sonic_assets.sprites._object._name) {                   \
            rbtk_unload_sprite(sonic_assets.sprites._object._name); \
            sonic_assets.sprites._object._name = NULL;              \
        }                                                           \
    } while(0)

#define sonic_load_sprite_anime(_object, _name,                 \
                                _frame_count, _duration, _unit) \
    do {                                                        \
        assert(_frame_count > 0);                               \
        assert(_duration > 0);                                  \
        if (sonic_assets.sprites._object._name) {               \
            break; /* animation already loaded */               \
        }                                                       \
                                                                \
        RBTK_SPRITE_ANIME *anime =                              \
            rbtk_create_sprite_anime((_frame_count));           \
        if(!anime) {                                            \
            sonic_assets.sprites._object._name = NULL;          \
            break; /* error creating animation */               \
        }                                                       \
                                                                \
        long double frame_duration =                            \
            (_duration) / (_frame_count);                       \
                                                                \
        char path[2048];                                        \
        for (size_t i = 0; i < (_frame_count); i++) {           \
            const char *format = "sprites/" #_object "/"        \
                #_name "/" #_name "_%d.png";                    \
            if (sprintf(path, format, i) < 0) {                 \
                rbtk_destroy_sprite_anime(anime, true);         \
                sonic_assets.sprites._object._name = NULL;      \
                break; /* error formatting string */            \
            }                                                   \
                                                                \
            RBTK_ASSET *asset = rbtk_require_asset(path);       \
            RBTK_SPRITE *sprite = rbtk_load_sprite(asset);      \
            if (!sprite) {                                      \
                rbtk_destroy_sprite_anime(anime, true);         \
                sonic_assets.sprites._object._name = NULL;      \
                break; /* sprite failed to load */              \
            }                                                   \
                                                                \
            if (!rbtk_add_sprite_to_anime(anime, sprite,        \
                    frame_duration, (_unit))) {                 \
                rbtk_destroy_sprite_anime(anime, true);         \
                sonic_assets.sprites._object._name = NULL;      \
                break; /* failed to add next frame */           \
            }                                                   \
        }                                                       \
                                                                \
        sonic_assets.sprites._object._name = anime;             \
    } while(0)

#define sonic_unload_sprite_anime(_object, _name)                          \
    do {                                                                   \
        if (sonic_assets.sprites._object._name) {                          \
            rbtk_destroy_sprite_anime(sonic_assets.sprites._object._name,  \
                true);                                                     \
            sonic_assets.sprites._object._name = NULL;                     \
        }                                                                  \
    } while(0)

#define sonic_buffer_sound(_category, _object, _name)                     \
    do {                                                                  \
        if (!sonic_assets._category._object._name) {                      \
	    const char *path = #_category "/" #_object "/" #_name ".ogg"; \
	    RBTK_ASSET *asset = rbtk_require_asset(path);                 \
                                                                          \
            RBTK_IN_STREAM *in = rbtk_open_asset_in_stream(asset);        \
            if (!in) {                                                    \
                sonic_assets._category._object._name = NULL;              \
                break; /* error opening input stream */                   \
            }                                                             \
                                                                          \
            RBTK_AUDIO_SOURCE *src = rbtk_source_ogg(in);                 \
            if (!src) {                                                   \
                sonic_assets._category._object._name = NULL;              \
                break; /* error sourcing OGG file */                      \
            }                                                             \
                                                                          \
            RBTK_SOUND *sound = rbtk_buffer_sound(src);                   \
            if (!sound) {                                                 \
                rbtk_close_audio_source(src);                             \
                rbtk_close_in_stream(in);                                 \
                sonic_assets._category._object._name = NULL;              \
                break; /* error buffering audio source */                 \
            }                                                             \
                                                                          \
            rbtk_close_in_stream(in);                                     \
                                                                          \
	    sonic_assets._category._object._name = sound;                 \
        }                                                                 \
    } while (0)

#define sonic_close_sound(_category, _object, _name)                \
    do {                                                            \
        if (sonic_assets._category._object._name) {                 \
            rbtk_close_sound(sonic_assets._category._object._name); \
            sonic_assets._category._object._name = NULL;            \
        }                                                           \
    } while(0)

#define sonic_buffer_ost(_object, _name)    \
    sonic_buffer_sound(ost, _object, _name)
#define sonic_close_ost(_object, _name)     \
    sonic_close_sound(ost, _object, _name)

#define sonic_buffer_sfx(_object, _name)    \
    sonic_buffer_sound(sfx, _object, _name)
#define sonic_close_sfx(_object, _name)     \
    sonic_close_sound(sfx, _object, _name)

typedef struct sonic_globals_type {
    RBTK_WINDOW *window;
    RBTK_GRAPHICS *scene;
    struct {
        RBTK_GAME_STATE *title;
        RBTK_GAME_STATE *load;
        RBTK_GAME_STATE *play;
    } states;
} sonic_globals_type;

typedef struct sonic_assets_type {
    struct {
        struct {
            RBTK_SOUND *present;
        } carbon_cavern;
        struct {
            RBTK_SOUND *title_theme_intro;
            RBTK_SOUND *title_theme_loop;
            RBTK_SOUND *title_theme_ym2612_intro;
            RBTK_SOUND *title_theme_ym2612_loop;
        } title;
    } ost;
    struct {
        struct {
            RBTK_SOUND *select;
        } menu;
    } sfx;
    struct {
        struct {
            RBTK_SPRITE *idle;
            RBTK_SPRITE_ANIME *motion;
        } sonic;
        struct {
            RBTK_SPRITE_ANIME *sonic_bust_appear;
            RBTK_SPRITE_ANIME *sonic_finger_wag;
            RBTK_SPRITE *banner;
            RBTK_SPRITE *bg;
            RBTK_SPRITE *black;
            RBTK_SPRITE *c_sega_1993;
            RBTK_SPRITE *clouds;
            RBTK_SPRITE *flash;
            RBTK_SPRITE *lake;
            RBTK_SPRITE *little_planet;
            RBTK_SPRITE *medal;
            RBTK_SPRITE *press_enter;
            RBTK_SPRITE *press_start;
            RBTK_SPRITE *sky;
            RBTK_SPRITE *sonic_bust;
            RBTK_SPRITE *sonic_bust_raised_eyebrow;
            RBTK_SPRITE *tm;
        } title;
    } sprites;
} sonic_assets_type;

extern sonic_globals_type sonic_globals;
extern sonic_assets_type sonic_assets;

extern const rbtk_game_funs sonic_game_funs;
extern const rbtk_game_state_funs sonic_title_state_funs;
extern const rbtk_game_state_funs sonic_load_state_funs;
extern const rbtk_game_state_funs sonic_play_state_funs;

#endif /* SONIC_GAME_H_ */

