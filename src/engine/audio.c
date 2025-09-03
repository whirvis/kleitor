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
#include "audio.h"
#include "./private/audio.h"
#include "./platform/audio.h"

#include <assert.h>
#include <math.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "../libraries/stb_vorbis.h"
#include "../libraries/minimp3_ex.h"

#include "../runtime/common.h"
#include "../runtime/error.h"

#define MIN_BUFSIZE 4096   /* usually just enough */
#define MAX_BUFSIZE 176400 /* 1s of 16-bit stereo */

static struct rbtk_maintained_sounds *maintained_head;
static struct rbtk_maintained_sounds *maintained_tail;
static bool initialized;

RBTK_PRIVATE RBTK_NO_DISCARD bool
priv_rbtk_audio_init(void)
{
    if (initialized) {
        return true;
    }

    if (!plat_rbtk_audio_init()) {
        return false;
    }

    maintained_head = NULL;
    maintained_tail = NULL;

    initialized = true;
    return true;
}

RBTK_PRIVATE RBTK_NO_DISCARD bool
priv_rbtk_audio_terminate(void)
{
    if (!initialized) {
        return true;
    }

    rbtk_maintained_sounds *cur = maintained_head;
    while (cur) {
        RBTK_SOUND *sound = cur->sound;
        rbtk_close_sound(sound);
        priv_rbtk_audio_abandon(sound);
        cur = maintained_head;
    }

    if (!plat_rbtk_audio_terminate()) {
        return false;
    }

    maintained_head = NULL;
    maintained_tail = NULL;

    initialized = false;
    return true;
}

RBTK_PRIVATE bool
priv_rbtk_audio_maintain(RBTK_SOUND *sound)
{
    assert(sound);
    assert(initialized);

    if (sound->maintained) {
        return true;
    }

    rbtk_maintained_sounds *elem = NULL;
    RBTK_MALLOC_OR_RETURN(&elem, false,
        "could not maintain another sound");

    elem->sound = sound;
    elem->next = NULL;
    elem->prev = NULL;

    RBTK_DLL_PUSH(maintained_head, maintained_tail, elem);
    sound->maintained = elem;

    return true;
}

RBTK_PRIVATE void
priv_rbtk_audio_abandon(RBTK_SOUND *sound)
{
    assert(sound);
    if (initialized && sound->maintained) {
        RBTK_DLL_REMOVE(maintained_head, maintained_tail,
            sound->maintained);
        free(sound->maintained);
        sound->maintained = NULL;
    }
}

typedef struct RBTK_AUDIO_SOURCE {
    rbtk_audio_source_funs funs;
    rbtk_audio_source_info info;
    void *impl;
} RBTK_AUDIO_SOURCE;

static bool
rbtk_no_op_close_audio_source(RBTK_UNUSED RBTK_AUDIO_SOURCE *src,
    RBTK_UNUSED void *impl)
{
    return true;
}

RBTK_AUDIO_SOURCE *
rbtk_source_audio(rbtk_audio_source_funs funs,
    rbtk_audio_source_info info, void *impl)
{
    assert(funs.close    != (rbtk_close_audio_source_fun) RBTK_DEFAULT_IMPL);
    assert(funs.close    != (rbtk_close_audio_source_fun) RBTK_UNIMPLEMENTED);
    assert(funs.read_pcm != (rbtk_read_pcm_fun)           RBTK_NO_OP);
    assert(funs.read_pcm != (rbtk_read_pcm_fun)           RBTK_DEFAULT_IMPL);
    assert(funs.read_pcm != (rbtk_read_pcm_fun)           RBTK_UNIMPLEMENTED);
    assert(impl);

    RBTK_AUDIO_SOURCE *src = NULL;
    RBTK_MALLOC_OR_RETURN(&src, NULL,
        "could not allocate audio source");

    if (funs.close == (rbtk_close_audio_source_fun) RBTK_NO_OP) {
        funs.close = rbtk_no_op_close_audio_source;
    }

    src->funs = funs;
    src->info = info;
    src->impl = impl;

    return src;
}

bool
rbtk_close_audio_source(RBTK_AUDIO_SOURCE *src)
{
    assert(src);
    if (!src->funs.close(src, src->impl)) {
        return false;
    }
    free(src);
    return true;
}

const rbtk_audio_source_info *
rbtk_get_audio_source_info(const RBTK_AUDIO_SOURCE *src)
{
    assert(src);
    return &src->info;
}

RBTK_NO_DISCARD int
rbtk_read_pcm(RBTK_AUDIO_SOURCE *src, size_t off, void *buf, size_t len)
{
    assert(src);
    assert(buf);

    int read = src->funs.read_pcm(src, src->impl, off, buf, len);
    assert(read < 0 || ((unsigned int) read) <= len); /* check for overrun */
    return read;
}

RBTK_NO_DISCARD RBTK_AUDIO_SOURCE *
rbtk_source_wav(RBTK_UNUSED RBTK_IN_STREAM *in) {
    assert(in);
    rbtk_signal_error(RBTK_ERROR_UNSUPPORTED,
        "sourcing WAVs not yet implemented");
    return NULL;
}

#define VORBIS_BITS_PER_SAMPLE  16
#define VORBIS_BYTES_PER_SAMPLE 2

#define PCM_FROM_VORBIS_SAMPLE(_sample) \
	rbtk_clamp_i16((short) (32768 * (_sample)), -32768, 32767)

/*!
 * @brief Doubles the size of a buffer.
 *
 * The contents of the current buffer are copied to the new buffer, at
 * which point the original buffer is freed.
 *
 * @note On error, both the contents `buf` and `cur_size` are left as-is.
 *
 * @param [in,out] buf      A pointer to the original buffer. This is read
 *                          from to copy the original contents to the newly
 *                          allocated buffer. Afterwards, it is written to
 *                          contain the location of the new buffer.
 * @param [in,out] cur_size A pointer to the current size of the buffer.
 *                          This is read from to determine how many bytes
 *                          to copy, before being written to contain the
 *                          new size.
 * @return The number of unused bytes in the newly allocated buffer or
 * `SIZE_MAX` on error.
 */
static size_t
double_buffer_size(unsigned char **buf, size_t *bufsize)
{
    assert(buf && *buf);
    assert(bufsize);

    void *prev_buf = *buf;
    size_t prev_size = *bufsize;

    size_t next_size = prev_size * 2;
    void *next_buf = malloc(next_size);
    if (!next_buf) {
        rbtk_signal_error(RBTK_ERROR_OUT_OF_MEMORY,
            "could not allocate memory for larger buffer");
        return SIZE_MAX;
    }

    memcpy(next_buf, prev_buf, prev_size);
    free(prev_buf); /* no longer needed */
    *buf = next_buf;
    *bufsize = next_size;

    /* unused bytes in new buffer */
    return next_size - prev_size;
}

/*!
 * @brief Shifts unused data from the end of the buffer to the front.
 *
 * @note The data at the end of the buffer will be left untouched, it is
 * assumed the caller will simply not read from it until they have written
 * over it with other data later.
 *
 * @param[in]     buf  The buffer whose data to shift.
 * @param[in,out] read A pointer to the number of bytes that have been read.
 *                     The value at this location will be updated to contain
 *                     the number of unused bytes now at the front of `buf`.
 * @param[in]     off  The offset to read and write from.
 */
static void
shift_unconsumed_to_front(void *buf, size_t *read, size_t consumed)
{
    unsigned char *cbuf = buf;
    size_t unconsumed = *read - consumed;
    memcpy(cbuf, cbuf + consumed, unconsumed);
    *read = unconsumed;
}

static RBTK_NO_DISCARD bool
open_vorbis_decoder(rbtk_vorbis_audio_source *vorbis)
{
    assert(vorbis);
    assert(vorbis->in);

    size_t bufsize = MIN_BUFSIZE;
    unsigned char *buf = malloc(bufsize);
    if (!buf) {
        rbtk_signal_error(RBTK_ERROR_OUT_OF_MEMORY,
            "could not allocate memory for Ogg Vorbis buffer");
        return false;
    }

    /*
     * Attempt an initial read of the data. The bytes read into the buffer
     * will be used by the STB Vorbis pushdata API in its attempt to open a
     * decoder. We use our own read offset here (as opposed to the one in
     * the Vorbis audio source structure) as we will be resizing the buffer
     * if necessary.
     */
    RBTK_IN_STREAM *in = vorbis->in;
    size_t read_offset = rbtk_read_bytes(in, buf, 0, bufsize);

    int bytes_consumed = 0;
    int vorbis_error = 0;
    stb_vorbis *decoder = NULL;

    while (!decoder) {
        decoder = stb_vorbis_open_pushdata(buf,
            (int) bufsize, &bytes_consumed, &vorbis_error, NULL);

        if (!decoder) {
            if (vorbis_error != VORBIS_need_more_data) {
                rbtk_signal_error(RBTK_ERROR_IO,
                    "Ogg Vorbis error %d", vorbis_error);
                return false;
            }

            if (bufsize >= MAX_BUFSIZE) {
                rbtk_signal_error(RBTK_ERROR_IO,
                    "Ogg Vorbis exceeded max buffer size");
                return false;
            }

            /*
             * The STB Vorbis pushdata API needs more memory in order to
             * open the decoder. Double the buffer size and read more data
             * from the stream to accomodate for this.
             */
            size_t unused = double_buffer_size(&buf, &bufsize);
            size_t read = rbtk_read_bytes(in, buf, read_offset, unused);
            read_offset += read;

            /*
             * If the stream says that it didn't read any bytes, we must
             * signal an error and return to avoid a possible softlock.
             */
            if (read < 0) {
                free(buf); /* will no longer be used */
                rbtk_signal_error(RBTK_ERROR_IO,
                    "could not open Ogg Vorbis decoder");
                return false;
            }
        }
    }

    shift_unconsumed_to_front(buf, &read_offset, bytes_consumed);

    vorbis->buffer = buf;
    vorbis->buffer_size = bufsize;
    vorbis->buffer_offset = read_offset;
    vorbis->decoder = decoder;
    vorbis->header_size = bytes_consumed;

    return true;
}

static RBTK_NO_DISCARD bool
close_vorbis_source(RBTK_UNUSED RBTK_AUDIO_SOURCE *src,
    rbtk_vorbis_audio_source *vorbis)
{
    assert(src);
    assert(vorbis);

    free(vorbis->buffer);
    stb_vorbis_close(vorbis->decoder);

    return true;
}

static void
write_vorbis_samples(rbtk_vorbis_audio_source *vorbis,
    size_t *bytes_written, void *buf, size_t len)
{
    assert(vorbis);
    assert(bytes_written);
    assert(buf);

    if (!vorbis->outputs) {
        return; /* nothing to write */
    }

    size_t channels = vorbis->channels;
    float **samples = vorbis->outputs;
    size_t sample_index = vorbis->outputs_index;
    size_t sample_count = vorbis->outputs_size;

    unsigned char *cbuf = buf;
    size_t channel_size = VORBIS_BYTES_PER_SAMPLE * channels;

    size_t off = *bytes_written;
    for (size_t s = sample_index; s < sample_count; s++) {
        if (off + channel_size > len) {
            break; /* no more room for samples */
        }
        for (size_t c = 0; c < channels; c++) {
            short pcm = PCM_FROM_VORBIS_SAMPLE(samples[c][s]);
            cbuf[off + 0] = (unsigned char) ((pcm & 0x00FF) >> 0);
            cbuf[off + 1] = (unsigned char) ((pcm & 0xFF00) >> 8);
            off += VORBIS_BYTES_PER_SAMPLE;
        }
        vorbis->outputs_index += 1;
    }
    *bytes_written = off;

    /*
     * If the output index is greater than or equal the output size, then
     * we have used all of the previous read samples. Clear the buffer so
     * the next read can make use of it.
     */
    if (vorbis->outputs_index >= vorbis->outputs_size) {
        vorbis->outputs = NULL;
        vorbis->outputs_size = 0;
        vorbis->outputs_index = 0;
    }
}

static RBTK_NO_DISCARD int
read_vorbis_pcm(RBTK_UNUSED RBTK_AUDIO_SOURCE *src,
    RBTK_UNUSED rbtk_vorbis_audio_source *vorbis, RBTK_UNUSED size_t off,
    void *buf, size_t len)
{
    assert(src);
    assert(vorbis);
    assert(buf);

    /* TODO: implement seeking */

    /*
     * Before reading any data, write any remaining samples which have been
     * obtained but not yet written. This usually occurs due to restrictions
     * with the buffer size.
     */
    size_t bytes_written = 0;
    write_vorbis_samples(vorbis, &bytes_written, buf, len);
    if (bytes_written >= len) {
        vorbis->expected_offset += bytes_written;
        return (int) bytes_written;
    }

    size_t read = rbtk_read_bytes(vorbis->in,
        vorbis->buffer + vorbis->buffer_offset, 0,
        vorbis->buffer_size - vorbis->buffer_offset);
    vorbis->buffer_offset += read;

    int samples = 0;
    while (samples == 0) {
        int bytes_used = stb_vorbis_decode_frame_pushdata(
            vorbis->decoder, vorbis->buffer,
            (int) vorbis->buffer_size, NULL,
            &vorbis->outputs, &samples);

        if (bytes_used == 0) {
            if (read <= 0) {
                return EOF; /* no more data left */
            }

            if (vorbis->buffer_size >= MAX_BUFSIZE) {
                rbtk_signal_error(RBTK_ERROR_IO,
                    "Ogg Vorbis exceeded max buffer size");
                return EOF;
            }

            size_t remaining = double_buffer_size(&vorbis->buffer, &vorbis->buffer_size);
            vorbis->buffer_offset += rbtk_read_bytes(vorbis->in, vorbis->buffer, vorbis->buffer_offset, remaining);
            continue; /* more data needed, try reading again */
        }

        assert(vorbis->buffer_offset >= (unsigned int) bytes_used);
        shift_unconsumed_to_front(vorbis->buffer, &vorbis->buffer_offset, bytes_used);
    }

    vorbis->outputs_index = 0;
    vorbis->outputs_size = samples;

    write_vorbis_samples(vorbis, &bytes_written, buf, len);

    if (bytes_written > 0) {
        vorbis->expected_offset += bytes_written;
        return (int) bytes_written;
    }
    else {
        return EOF;
    }
}

RBTK_NO_DISCARD RBTK_AUDIO_SOURCE *
rbtk_source_ogg(RBTK_IN_STREAM *in)
{
    assert(in);

    rbtk_vorbis_audio_source *vorbis = NULL;
    RBTK_MALLOC_OR_RETURN(&vorbis, NULL,
        "could not allocate Ogg Vorbis audio source");

    vorbis->in = in;

    vorbis->buffer = NULL;
    vorbis->buffer_size = 0;
    vorbis->buffer_offset = 0;
    vorbis->decoder = NULL;

    vorbis->expected_offset = 0;

    vorbis->channels = 0;
    vorbis->outputs = NULL;
    vorbis->outputs_index = 0;
    vorbis->outputs_size = 0;

    if (!open_vorbis_decoder(vorbis)) {
        free(vorbis);
        return NULL;
    }

    rbtk_audio_source_funs funs = {
            .close    = (rbtk_close_audio_source_fun) close_vorbis_source,
            .read_pcm = (rbtk_read_pcm_fun)           read_vorbis_pcm
    };

    stb_vorbis_info vorbis_info = stb_vorbis_get_info(vorbis->decoder);
    rbtk_audio_source_info info = {
            .frequency_hz = vorbis_info.sample_rate,
            .channel_count = vorbis_info.channels,
            .bits_per_sample = VORBIS_BITS_PER_SAMPLE
    };

    vorbis->channels = vorbis_info.channels;

    return rbtk_source_audio(funs, info, vorbis);
}

RBTK_NO_DISCARD RBTK_AUDIO_SOURCE *
rbtk_source_mp3(RBTK_UNUSED RBTK_IN_STREAM *in) {
    assert(in);
    rbtk_signal_error(RBTK_ERROR_UNSUPPORTED,
        "sourcing MP3s not yet implemented");
    return NULL;
}

#define PCM_BUFFER_CHUNK_SIZE 1024

typedef struct buffer_chunk {
    size_t len;
    unsigned char data[PCM_BUFFER_CHUNK_SIZE];
    struct buffer_chunk *next;
} buffer_chunk;

static unsigned char *
buffer_pcm_data(RBTK_AUDIO_SOURCE *src, size_t *pcm_buffer_size)
{
    assert(src);
    assert(pcm_buffer_size);

    bool malloc_failure = false;

    buffer_chunk *chunks_head = NULL;
    buffer_chunk *chunks_tail = NULL;
    size_t chunk_count = 0;

    size_t size = 0;
    unsigned char data[PCM_BUFFER_CHUNK_SIZE];

    /*
     * Before we can load the entire buffer into memory, we must first
     * get the PCM data in chunks. This will allow us to read all data
     * and also determine the total size before allocating the PCM buffer.
     * The chunks will be free'd as they are copied to the final buffer.
     */
    int read = rbtk_read_pcm(src, size, data, PCM_BUFFER_CHUNK_SIZE);
    while (read != EOF) {
        assert(read <= PCM_BUFFER_CHUNK_SIZE);

        buffer_chunk *chunk = malloc(sizeof(*chunk));
        if (!chunk) {
            rbtk_signal_error(RBTK_ERROR_OUT_OF_MEMORY,
                "could not allocate chunks for PCM buffer");
            malloc_failure = true;
            break; /* abort buffering */
        }

        chunk->len = read;
        memcpy(chunk->data, data, read);
        size += read;

        chunk->next = NULL;
        if (!chunks_head) {
            chunks_head = chunk;
            chunks_tail = chunk;
        }
        else {
            chunks_tail->next = chunk;
            chunks_tail = chunk;
        }
        chunk_count += 1;

        read = rbtk_read_pcm(src, size, data, PCM_BUFFER_CHUNK_SIZE);
    }

    /*
     * Now that we've loaded the chunks into memory, we can allocate a single
     * block of memory to store the PCM data into one contiguous buffer. This
     * means that for a time, twice the memory of the original PCM data will
     * be on the heap. However, each chunk will be free'd once their contents
     * have been copied to this buffer.
     */
    unsigned char *pcm_buffer = NULL;
    if (!malloc_failure) {
        pcm_buffer = malloc(size);
        if (!pcm_buffer) {
            rbtk_signal_error(RBTK_ERROR_OUT_OF_MEMORY,
                "could not allocate %d-byte PCM buffer", size);
            malloc_failure = true;
        }
    }

    /*
     * In the event of a failed allocation, we must ensure that we free all
     * allocated data. Failing to do so would result in a possibly monstrous
     * memory leak. This usually occurs when the audio file is too large to
     * buffer into memory all at once.
     */
    if (malloc_failure) {
        buffer_chunk *chunk = chunks_head;
        while (chunk) {
            buffer_chunk *next = chunk->next;
            free(chunk);
            chunk = next;
        }
        free(pcm_buffer);
        return NULL;
    }

    /*
     * Now we can commence the final step of copying the chunks into the final
     * PCM buffer (and freeing them immediately afterwards). Once this is done,
     * a final sanity check is done and the final PCM buffer size is written.
     */
    size_t pcm_offset = 0;
    buffer_chunk *chunk = chunks_head;
    while (chunk) {
        /* copy the current chunk to the PCM buffer */
        memcpy(pcm_buffer + pcm_offset, chunk->data, chunk->len);
        pcm_offset += chunk->len;

        /* free the current chunk, we don't need it */
        buffer_chunk *next = chunk->next;
        free(chunk);
        chunk = next;
    }

    assert(pcm_offset == size);
    *pcm_buffer_size = size;
    return pcm_buffer;
}

RBTK_NO_DISCARD RBTK_SOUND *
rbtk_buffer_sound(RBTK_AUDIO_SOURCE *src)
{
    assert(src);

    RBTK_SOUND *sound = NULL;
    RBTK_MALLOC_OR_RETURN(&sound, NULL,
        "could not allocate sound for audio source");

    PLAT_RBTK_SOUND *plat_sound = plat_rbtk_alloc_sound();
    if (!plat_sound) {
        free(sound);
        rbtk_suggest_error(RBTK_ERROR_OUT_OF_MEMORY,
            "could not allocate platform specific memory");
        return NULL;
    }

    /*
     * Now that we have our sound memory allocated, we must buffer all
     * of the PCM data into memory. This will be taken in by a platform
     * specific implementation which will use the buffered data. This
     * PCM buffer will then be freed immediately afterwards.
     */
    size_t pcm_buffer_size = 0;
    void *pcm_buffer = buffer_pcm_data(src, &pcm_buffer_size);
    if (!pcm_buffer) {
        free(sound);
        free(plat_sound);
        return NULL;
    }

    sound->plat = plat_sound;
    sound->src = src;
    sound->type = RBTK_SOUND_TYPE_BUFFERED;
    sound->looping = false;
    sound->closed = false;
    sound->maintained = NULL;

    plat_rbtk_buffer_sound(sound, pcm_buffer_size, pcm_buffer);
    free(pcm_buffer); /* we don't need this anymore */
    priv_rbtk_audio_maintain(sound);

    return sound;
}

RBTK_NO_DISCARD RBTK_SOUND *
rbtk_stream_sound(RBTK_UNUSED RBTK_AUDIO_SOURCE *src)
{
    assert(src);
    rbtk_signal_error(RBTK_ERROR_UNSUPPORTED,
        "streamed sounds not yet implemented");
    return NULL;
}

void
rbtk_close_sound(RBTK_SOUND *sound)
{
    assert(sound);
    if (!sound->closed) {
        plat_rbtk_close_sound(sound);
        priv_rbtk_audio_abandon(sound);
        sound->closed = true;
    }
}

RBTK_NO_DISCARD rbtk_sound_state
rbtk_get_sound_state(const RBTK_SOUND *sound)
{
    assert(sound);
    return plat_rbtk_get_sound_state(sound);
}

void
rbtk_set_sound_volume(RBTK_SOUND *sound, float volume)
{
    assert(sound);
    float clamped = rbtk_clamp_f32(volume, 0.0f, 1.0f);
    plat_rbtk_set_sound_volume(sound, clamped);
}

void
rbtk_increase_volume(RBTK_SOUND *sound, float amount)
{
    assert(sound);
    float current = rbtk_get_sound_volume(sound);
    rbtk_set_sound_volume(sound, current + amount);
}

void
rbtk_decrease_volume(RBTK_SOUND *sound, float amount)
{
    assert(sound);
    float current = rbtk_get_sound_volume(sound);
    rbtk_set_sound_volume(sound, current - amount);
}

float
rbtk_get_sound_volume(const RBTK_SOUND *sound)
{
    assert(sound);
    return plat_rbtk_get_sound_volume(sound);
}

void
rbtk_play_sound(RBTK_SOUND *sound)
{
    assert(sound);
    plat_rbtk_play_sound(sound);
}

void
rbtk_pause_sound(RBTK_SOUND *sound)
{
    assert(sound);
    plat_rbtk_pause_sound(sound);
}

void
rbtk_stop_sound(RBTK_SOUND *sound)
{
    assert(sound);
    plat_rbtk_stop_sound(sound);
}

RBTK_NO_DISCARD bool
rbtk_sound_is_looping(const RBTK_SOUND *sound)
{
    assert(sound);
    return sound->looping;
}

void
rbtk_loop_sound(RBTK_SOUND *sound, bool looping)
{
    assert(sound);
    plat_rbtk_loop_sound(sound, looping);
    sound->looping = looping;
}

RBTK_NO_DISCARD long double
rbtk_get_sound_offset(const RBTK_SOUND *sound, rbtk_time_unit guide)
{
    assert(sound);
    return plat_rbtk_get_sound_offset(sound, guide);
}

void
rbtk_set_sound_offset(RBTK_SOUND *sound, rbtk_time_unit guide,
    long double offset)
{
    assert(sound);
    assert(offset >= 0);
    plat_rbtk_set_sound_offset(sound, guide, offset);
}

void
rbtk_skip_sound(RBTK_SOUND *sound, rbtk_time_unit guide,
    long double offset)
{
    assert(sound);
    long double current_offset = rbtk_get_sound_offset(sound, guide);
    long double updated_offset = current_offset + offset;
    if (updated_offset < 0.0l) {
        updated_offset = 0.0l;
    }
    rbtk_set_sound_offset(sound, guide, updated_offset);
}
