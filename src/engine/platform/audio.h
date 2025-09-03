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
#ifndef RBTK_ENGINE_PLATFORM_AUDIO_H_
#define RBTK_ENGINE_PLATFORM_AUDIO_H_

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "../audio.h"
#include "../private/audio.h"

#include "../../runtime/common.h"

RBTK_FORWARD_DECLARATION
typedef struct PLAT_RBTK_SOUND PLAT_RBTK_SOUND;

RBTK_PLATFORM RBTK_NO_DISCARD bool
plat_rbtk_audio_init(void);

RBTK_PLATFORM RBTK_NO_DISCARD bool
plat_rbtk_audio_terminate(void);

RBTK_PLATFORM RBTK_NO_DISCARD PLAT_RBTK_SOUND *
plat_rbtk_alloc_sound(void);

RBTK_PLATFORM void
plat_rbtk_buffer_sound(RBTK_SOUND *sound, size_t pcm_buffer_size,
    void *pcm_buffer);

RBTK_PLATFORM void
plat_rbtk_close_sound(RBTK_SOUND *sound);

RBTK_PLATFORM RBTK_NO_DISCARD rbtk_sound_state
plat_rbtk_get_sound_state(const RBTK_SOUND *sound);

void
plat_rbtk_set_sound_volume(RBTK_SOUND *sound, float volume);

float
plat_rbtk_get_sound_volume(const RBTK_SOUND *sound);

RBTK_PLATFORM void
plat_rbtk_play_sound(RBTK_SOUND *sound);

RBTK_PLATFORM void
plat_rbtk_pause_sound(RBTK_SOUND *sound);

RBTK_PLATFORM void
plat_rbtk_stop_sound(RBTK_SOUND *sound);

RBTK_PLATFORM void
plat_rbtk_loop_sound(RBTK_SOUND *sound, bool looping);

RBTK_PLATFORM RBTK_NO_DISCARD long double
plat_rbtk_get_sound_offset(const RBTK_SOUND *sound, rbtk_time_unit unit);

RBTK_PLATFORM void
plat_rbtk_set_sound_offset(RBTK_SOUND *sound, rbtk_time_unit unit,
    long double offset);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* RBTK_ENGINE_PLATFORM_AUDIO_H_ */
