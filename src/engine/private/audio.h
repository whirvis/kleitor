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
#ifndef RBTK_ENGINE_PRIVATE_AUDIO_H_
#define RBTK_ENGINE_PRIVATE_AUDIO_H_

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "../audio.h"

#include <stdbool.h>

#include "../../runtime/common.h"

RBTK_FORWARD_DECLARATION
typedef struct PLAT_RBTK_SOUND PLAT_RBTK_SOUND;

typedef enum RBTK_SOUND_TYPE {
    RBTK_SOUND_TYPE_BUFFERED,
    RBTK_SOUND_TYPE_STREAMED
} RBTK_SOUND_TYPE;

typedef struct rbtk_maintained_sounds {
    RBTK_SOUND *sound;
	struct rbtk_maintained_sounds *prev;
    struct rbtk_maintained_sounds *next;
} rbtk_maintained_sounds;

typedef struct RBTK_SOUND {
    PLAT_RBTK_SOUND *plat;
    const RBTK_AUDIO_SOURCE *src;
    RBTK_SOUND_TYPE type;
    bool looping;
    bool closed;
    rbtk_maintained_sounds *maintained;
} RBTK_SOUND;

RBTK_PRIVATE RBTK_NO_DISCARD bool
priv_rbtk_audio_init(void);

RBTK_PRIVATE RBTK_NO_DISCARD bool
priv_rbtk_audio_terminate(void);

RBTK_PRIVATE bool
priv_rbtk_audio_maintain(RBTK_SOUND *sound);

RBTK_PRIVATE void
priv_rbtk_audio_abandon(RBTK_SOUND *sound);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* RBTK_ENGINE_PRIVATE_AUDIO_H_ */
