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
#if defined(_WIN32) || defined(__linux__)

#include "audio.h"

#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "../../runtime/error.h"

#include <AL/al.h>
#include <AL/alc.h>

typedef struct PLAT_RBTK_SOUND {
    ALuint al_source;
    union {
        struct {
            ALuint al_buffer;
        } buffered;
        struct {
            ALuint al_buffers[2];
        } streamed;
    };
} PLAT_RBTK_SOUND;

static ALCdevice *device;
static ALCcontext *context;
static bool initialized;

RBTK_PLATFORM RBTK_NO_DISCARD bool
plat_rbtk_audio_init(void)
{
    if (initialized) {
        return true;
    }

    device = alcOpenDevice(NULL);
    if (!device) {
        rbtk_signal_error(RBTK_ERROR_PLATFORM,
            "failed to open AL device");
        return false;
    }

    context = alcCreateContext(device, NULL);
    if (!context) {
        rbtk_signal_error(RBTK_ERROR_PLATFORM,
            "failed to create AL context");
        return false;
    }

    if (!alcMakeContextCurrent(context)) {
        rbtk_signal_error(RBTK_ERROR_PLATFORM,
            "failed to make AL context current");
        return false;
    }

    initialized = true;
    return true;
}

RBTK_PLATFORM RBTK_NO_DISCARD bool
plat_rbtk_audio_terminate(void)
{
    if (!initialized) {
        return true;
    }

    alcMakeContextCurrent(NULL);
    alcDestroyContext(context);
    alcCloseDevice(device);

    initialized = false;
    return true;
}

RBTK_PLATFORM RBTK_NO_DISCARD PLAT_RBTK_SOUND *
plat_rbtk_alloc_sound(void)
{
    PLAT_RBTK_SOUND *plat_sound = malloc(sizeof(*plat_sound));
    return plat_sound;
}

static ALint
get_al_format(const rbtk_audio_source_info *info) {
    assert(info);
    if (info->channel_count == 1) {
        return AL_FORMAT_MONO16;
    }
    else if (info->channel_count == 2) {
        return AL_FORMAT_STEREO16;
    }
    return -1;
}

RBTK_PLATFORM void
plat_rbtk_buffer_sound(RBTK_SOUND *sound, size_t pcm_buffer_size,
    void *pcm_buffer)
{
    assert(sound);
    assert(pcm_buffer);

    PLAT_RBTK_SOUND *plat = sound->plat;
    const RBTK_AUDIO_SOURCE *src = sound->src;
    const rbtk_audio_source_info *info = rbtk_get_audio_source_info(src);
    ALint al_format = get_al_format(info);

    alGenSources(1, &plat->al_source);
    alGenBuffers(1, &plat->buffered.al_buffer);

    alBufferData(plat->buffered.al_buffer, al_format, pcm_buffer,
        (ALsizei) pcm_buffer_size, info->frequency_hz);
    alSourcei(plat->al_source, AL_BUFFER, plat->buffered.al_buffer);
}

RBTK_PLATFORM void
plat_rbtk_close_sound(RBTK_SOUND *sound)
{
    assert(sound);

    PLAT_RBTK_SOUND *plat = sound->plat;

    alDeleteSources(1, &plat->al_source);

    if (sound->type == RBTK_SOUND_TYPE_BUFFERED) {
        ALuint al_buffer = plat->buffered.al_buffer;
        alDeleteBuffers(1, &al_buffer);
    }
    else if (sound->type == RBTK_SOUND_TYPE_STREAMED) {
        assert(0); /* TODO: implement this */
    }
    else {
        assert(0); /* unexpected type */
    }
}

RBTK_PLATFORM RBTK_NO_DISCARD rbtk_sound_state
plat_rbtk_get_sound_state(const RBTK_SOUND *sound)
{
    assert(sound);

    PLAT_RBTK_SOUND *plat = sound->plat;
    ALint state = 0;
    alGetSourcei(plat->al_source, AL_SOURCE_STATE, &state);

    switch (state) {
    case AL_INITIAL:
        return RBTK_SOUND_STATE_STOPPED;
    case AL_PAUSED:
        return RBTK_SOUND_STATE_PAUSED;
    case AL_PLAYING:
        return RBTK_SOUND_STATE_PLAYING;
    case AL_STOPPED:
        return RBTK_SOUND_STATE_STOPPED;
    default:
        assert(0);                       /* we forgot one!      */
        return RBTK_SOUND_STATE_STOPPED; /* pacify the compiler */
    }
}

void
plat_rbtk_set_sound_volume(RBTK_SOUND *sound, float volume)
{
    assert(sound);
    PLAT_RBTK_SOUND *plat = sound->plat;
    alSourcef(plat->al_source, AL_GAIN, volume);
}

float
plat_rbtk_get_sound_volume(const RBTK_SOUND *sound)
{
    assert(sound);
    PLAT_RBTK_SOUND *plat = sound->plat;
    ALfloat gain;
    alGetSourcef(plat->al_source, AL_GAIN, &gain);
    return gain;
}

RBTK_PLATFORM void
plat_rbtk_play_sound(RBTK_SOUND *sound)
{
    assert(sound);
    PLAT_RBTK_SOUND *plat = sound->plat;
    alSourcePlay(plat->al_source);
}

RBTK_PLATFORM void
plat_rbtk_pause_sound(RBTK_SOUND *sound)
{
    assert(sound);
    PLAT_RBTK_SOUND *plat = sound->plat;
    alSourcePause(plat->al_source);
}

RBTK_PLATFORM void
plat_rbtk_stop_sound(RBTK_SOUND *sound)
{
    assert(sound);
    PLAT_RBTK_SOUND *plat = sound->plat;
    alSourceStop(plat->al_source);
}

RBTK_PLATFORM void
plat_rbtk_loop_sound(RBTK_SOUND *sound, bool looping)
{
    assert(sound);
    PLAT_RBTK_SOUND *plat = sound->plat;
    alSourcei(plat->al_source, AL_LOOPING, looping);
}

RBTK_PLATFORM RBTK_NO_DISCARD long double
plat_rbtk_get_sound_offset(const RBTK_SOUND *sound, rbtk_time_unit unit)
{
    assert(sound);

    PLAT_RBTK_SOUND *plat = sound->plat;

    ALfloat offset;
    alGetSourcef(plat->al_source, AL_SEC_OFFSET, &offset);
    return rbtk_convert_time(RBTK_SECS, unit, offset);
}

RBTK_PLATFORM void
plat_rbtk_set_sound_offset(RBTK_SOUND *sound, rbtk_time_unit unit, long double offset)
{
    assert(sound);
    assert(offset >= 0);

    PLAT_RBTK_SOUND *plat = sound->plat;
    long double secs = rbtk_convert_time(unit, RBTK_SECS, offset);
    alSourcef(plat->al_source, AL_SEC_OFFSET, (ALfloat) secs);
}

#endif /* defined(_WIN32) || defined(__linux__) */
