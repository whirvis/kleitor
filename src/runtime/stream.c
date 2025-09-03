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
#include "stream.h"
#include "./private/stream.h"
#include "./platform/stream.h"

#include <assert.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"

typedef struct RBTK_IN_STREAM {
    rbtk_in_stream_funs funs;
    void *src;
} RBTK_IN_STREAM;

static bool
rbtk_no_op_close_in_stream(RBTK_UNUSED RBTK_IN_STREAM *in,
    RBTK_UNUSED void *src)
{
    return true;
}

static size_t
rbtk_unimplemented_available_bytes(RBTK_UNUSED RBTK_IN_STREAM *in,
    RBTK_UNUSED void *src)
{
    rbtk_signal_error(RBTK_ERROR_UNSUPPORTED,
        "rbtk_available_bytes() not implemented for this stream");
    return SIZE_MAX;
}

static size_t
rbtk_default_read_bytes(RBTK_IN_STREAM *in,
    RBTK_UNUSED void *src, void *buf, size_t off, size_t len)
{
    unsigned char *buf_bytes = buf;
    size_t read = 0;
    for (size_t i = 0; i < len; i++) {
        short next = rbtk_read_byte(in);
        if (next < 0) {
            break;
        }
        buf_bytes[off + i] = (unsigned char) next;
        read += 1;
    }
    return read;
}

static size_t
rbtk_default_skip_bytes(RBTK_IN_STREAM *in,
    RBTK_UNUSED void *src, size_t amt)
{
    size_t skipped = 0;
    for (size_t i = 0; i < amt; i++) {
        if (rbtk_read_byte(in) < 0) {
            break;
        }
        skipped += 1;
    }
    return skipped;
}

static size_t
rbtk_unimplemented_seek_to(RBTK_UNUSED RBTK_IN_STREAM *in,
    RBTK_UNUSED void *src, RBTK_UNUSED size_t pos)
{
    rbtk_signal_error(RBTK_ERROR_UNSUPPORTED,
        "rbtk_seek_to() not implemented for this stream");
    return SIZE_MAX;
}

RBTK_NO_DISCARD RBTK_IN_STREAM *
rbtk_open_in_stream(rbtk_in_stream_funs funs, void *src)
{
    assert(funs.close           != (rbtk_in_stream_close_fun)           RBTK_DEFAULT_IMPL);
    assert(funs.close           != (rbtk_in_stream_close_fun)           RBTK_UNIMPLEMENTED);
    assert(funs.available_bytes != (rbtk_in_stream_available_bytes_fun) RBTK_NO_OP);
    assert(funs.available_bytes != (rbtk_in_stream_available_bytes_fun) RBTK_DEFAULT_IMPL);
    assert(funs.read_byte       != (rbtk_in_stream_read_byte_fun)       RBTK_NO_OP);
    assert(funs.read_byte       != (rbtk_in_stream_read_byte_fun)       RBTK_DEFAULT_IMPL);
    assert(funs.read_byte       != (rbtk_in_stream_read_byte_fun)       RBTK_UNIMPLEMENTED);
    assert(funs.read_bytes      != (rbtk_in_stream_read_bytes_fun)      RBTK_NO_OP);
    assert(funs.read_bytes      != (rbtk_in_stream_read_bytes_fun)      RBTK_UNIMPLEMENTED);
    assert(funs.skip_bytes      != (rbtk_in_stream_skip_bytes_fun)      RBTK_NO_OP);
    assert(funs.skip_bytes      != (rbtk_in_stream_skip_bytes_fun)      RBTK_UNIMPLEMENTED);
    assert(funs.seek_to         != (rbtk_in_stream_seek_to_fun)         RBTK_NO_OP);
    assert(funs.seek_to         != (rbtk_in_stream_seek_to_fun)         RBTK_DEFAULT_IMPL);
    assert(src);

    RBTK_IN_STREAM *in = NULL;
    RBTK_MALLOC_OR_RETURN(&in, NULL, NULL);

    if (!funs.close) {
        funs.close = rbtk_no_op_close_in_stream;
    }

    if (funs.available_bytes == (rbtk_in_stream_available_bytes_fun) RBTK_UNIMPLEMENTED) {
        funs.available_bytes = rbtk_unimplemented_available_bytes;
    }
    if (funs.read_bytes == (rbtk_in_stream_read_bytes_fun) RBTK_DEFAULT_IMPL) {
        funs.read_bytes = rbtk_default_read_bytes;
    }
    if (funs.skip_bytes == (rbtk_in_stream_skip_bytes_fun) RBTK_DEFAULT_IMPL) {
        funs.skip_bytes = rbtk_default_skip_bytes;
    }
    if (funs.seek_to == (rbtk_in_stream_seek_to_fun) RBTK_UNIMPLEMENTED) {
        funs.seek_to = rbtk_unimplemented_seek_to;
    }

    in->funs = funs;
    in->src = src;

    return in;
}

bool
rbtk_close_in_stream(RBTK_IN_STREAM *in)
{
    assert(in);
    if (!in->funs.close(in, in->src)) {
        return false;
    }
    free(in);
    return true;
}

RBTK_NO_DISCARD bool
rbtk_supports_available_bytes(RBTK_IN_STREAM *in)
{
    assert(in);
    return in->funs.available_bytes != rbtk_unimplemented_available_bytes;
}

RBTK_NO_DISCARD size_t
rbtk_available_bytes(RBTK_IN_STREAM *in)
{
    assert(in);
    return in->funs.available_bytes(in, in->src);
}

RBTK_NO_DISCARD short
rbtk_read_byte(RBTK_IN_STREAM *in)
{
    assert(in);
    return in->funs.read_byte(in, in->src);
}

RBTK_NO_DISCARD size_t
rbtk_read_bytes(RBTK_IN_STREAM *in, void *buf, size_t off, size_t len)
{
    assert(in && buf);
    return in->funs.read_bytes(in, in->src, buf, off, len);
}

#define BUFFER_CHUNK_DATA_SIZE 1024

typedef struct buffer_chunk {
    size_t len;
    unsigned char data[BUFFER_CHUNK_DATA_SIZE];
    struct buffer_chunk *next;
} buffer_chunk;

RBTK_NO_DISCARD void *
rbtk_buffer_remaining(RBTK_IN_STREAM *in, size_t *size)
{
    assert(in);
    assert(size);

    bool malloc_failure = false;

    buffer_chunk *chunks_head = NULL;
    buffer_chunk *chunks_tail = NULL;
    size_t chunk_count = 0;

    size_t data_size = 0;
    unsigned char data[BUFFER_CHUNK_DATA_SIZE];

    /*
     * Before we can load the entire buffer into memory, we must first
     * get the stream's data in chunks. This will allow us to read all
     * data and determine the total size before allocating the buffer.
     * The chunks will be free'd as they are copied to the final buffer.
     */
    size_t read = rbtk_read_bytes(in, data, 0, BUFFER_CHUNK_DATA_SIZE);
    while (read > 0) {
        assert(read <= BUFFER_CHUNK_DATA_SIZE);

        buffer_chunk *chunk = malloc(sizeof(*chunk));
        if (!chunk) {
            rbtk_signal_error(RBTK_ERROR_OUT_OF_MEMORY,
                "could not allocate chunks for buffered contents");
            malloc_failure = true;
            break; /* abort buffering */
        }

        chunk->len = read;
        memcpy(chunk->data, data, read);
        data_size += read;

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

        read = rbtk_read_bytes(in, data, 0, BUFFER_CHUNK_DATA_SIZE);
    }

    /*
     * Now that we've loaded the chunks into memory, we can allocate a single
     * block of memory to store the stream's data into one contiguous buffer.
     * This means that for a time, twice the memory of the original data will
     * be on the heap. However, each chunk will be free'd once their contents
     * have been copied to this buffer.
     */
    unsigned char *data_buffer = NULL;
    if (!malloc_failure) {
        data_buffer = malloc(data_size);
        if (!data_buffer) {
            rbtk_signal_error(RBTK_ERROR_OUT_OF_MEMORY,
                "could not allocate %d-byte PCM buffer", data_size);
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
        free(data_buffer);
        return NULL;
    }

    /*
     * Now we can commence the final step of copying the chunks into the
     * final buffer (and freeing them immediately afterwards). After, one
     * last sanity check is done and the final buffer size is written.
     */
    size_t data_offset = 0;
    buffer_chunk *chunk = chunks_head;
    while (chunk) {
        /* copy the current chunk to the data buffer */
        memcpy(data_buffer + data_offset, chunk->data, chunk->len);
        data_offset += chunk->len;

        /* free the current chunk, we don't need it */
        buffer_chunk *next = chunk->next;
        free(chunk);
        chunk = next;
    }

    assert(data_offset == data_size);
    *size = data_size;
    return data_buffer;
}

size_t
rbtk_skip_bytes(RBTK_IN_STREAM *in, size_t amt)
{
    assert(in);
    return in->funs.skip_bytes(in, in->src, amt);
}

RBTK_NO_DISCARD bool
rbtk_supports_seek(RBTK_IN_STREAM *in)
{
    assert(in);
    return in->funs.seek_to != rbtk_unimplemented_seek_to;
}

size_t
rbtk_seek_to(RBTK_IN_STREAM *in, size_t pos)
{
    assert(in);
    return in->funs.seek_to(in, in->src, pos);
}

RBTK_NO_DISCARD RBTK_IN_STREAM *
rbtk_open_file_in_stream(const char *filepath)
{
    assert(filepath);

    rbtk_file_in_stream_src *src = NULL;
    RBTK_MALLOC_OR_RETURN(&src, NULL, NULL);

    FILE *file = fopen(filepath, "rb");
    if (!file) {
        free(src);
        rbtk_signal_error(RBTK_ERROR_IO,
            "call to fopen() failed, does the file exist?");
        return NULL;
    }

    src->file = file;

    RBTK_IN_STREAM *in = rbtk_open_in_stream(
        rbtk_file_in_stream_funs, src);
    if (!in) {
        free(src);
        return NULL;
    }
    return in;
}

bool
rbtk_close_file_in_stream(RBTK_UNUSED RBTK_IN_STREAM *in,
    rbtk_file_in_stream_src *src)
{
    assert(in && src);
    if (fclose(src->file)) {
        rbtk_signal_error(RBTK_ERROR_IO, "call to fclose() failed");
        return false;
    }
    return true;
}

RBTK_NO_DISCARD size_t
rbtk_available_bytes_in_file_stream(RBTK_UNUSED RBTK_IN_STREAM *in,
    rbtk_file_in_stream_src *src)
{
    assert(in && src);
    long current_pos = ftell(src->file);
    if (current_pos < 0) {
        rbtk_signal_error(RBTK_ERROR_IO, "call to ftell() failed");
        return SIZE_MAX;
    }
    return (size_t) src->size - current_pos;
}

RBTK_NO_DISCARD short
rbtk_read_file_stream_byte(RBTK_UNUSED RBTK_IN_STREAM *in,
    rbtk_file_in_stream_src *src)
{
    assert(in && src);
    unsigned char buf;
    if (fread(&buf, sizeof(buf), 1, src->file) <= 0) {
        return EOF;
    }
    return buf;
}

RBTK_NO_DISCARD size_t
rbtk_read_file_stream_bytes(RBTK_UNUSED RBTK_IN_STREAM *in,
    rbtk_file_in_stream_src *src, void *buf, size_t off, size_t len)
{
    assert(in && src && buf);
    unsigned char *buf_bytes = buf;
    size_t read = fread(buf_bytes + off,
    	sizeof(unsigned char), len, src->file);
    memset(buf_bytes + off + read, 0x00, len - read);

    return read;
}

size_t
rbtk_skip_file_stream_bytes(RBTK_UNUSED RBTK_IN_STREAM *in,
    rbtk_file_in_stream_src *src, size_t amt)
{
    assert(in && src);

    /* prevent overrun */
    long current_pos = ftell(src->file);
    if (current_pos < 0) {
        rbtk_signal_error(RBTK_ERROR_IO, "call to ftell() failed");
        return SIZE_MAX;
    }
    if (current_pos + amt >= src->size) {
        amt = src->size - current_pos;
    }

    if (fseek(src->file, (long) amt, SEEK_CUR)) {
        rbtk_signal_error(RBTK_ERROR_IO, "call to fseek() failed");
        return SIZE_MAX;
    }

    return amt;
}

size_t
rbtk_seek_file_stream_to(RBTK_UNUSED RBTK_IN_STREAM *in,
    rbtk_file_in_stream_src *src, size_t pos)
{
    assert(in && src);

    /* prevent overrun */
    size_t origin = pos;
    if (origin >= src->size) {
        origin = src->size;
    }

    if (fseek(src->file, (long) origin, SEEK_SET)) {
        rbtk_signal_error(RBTK_ERROR_IO, "call to fseek() failed");
        return SIZE_MAX;
    }

    long new_pos = ftell(src->file);
    if (new_pos < 0) {
        rbtk_signal_error(RBTK_ERROR_IO, "call to ftell() failed");
        return SIZE_MAX;
    }

    return (size_t) new_pos;
}

RBTK_NO_DISCARD RBTK_IN_STREAM *
rbtk_open_memory_in_stream(void *addr, size_t len) {
    assert(addr);

    rbtk_memory_in_stream_src *src = NULL;
    RBTK_MALLOC_OR_RETURN(&src, NULL, NULL);

    src->addr = addr;
    src->len = len;
    src->pos = 0;

    RBTK_IN_STREAM *in = rbtk_open_in_stream(
        rbtk_memory_in_stream_funs, src);
    if (!in) {
        free(src);
        return NULL;
    }
    return in;
}

RBTK_NO_DISCARD size_t
rbtk_available_bytes_in_memory_stream(RBTK_UNUSED RBTK_IN_STREAM *in,
    rbtk_memory_in_stream_src *src)
{
    assert(in && src);
    return src->len - src->pos;
}

RBTK_NO_DISCARD short
rbtk_read_memory_stream_byte(RBTK_UNUSED RBTK_IN_STREAM *in,
    rbtk_memory_in_stream_src *src)
{
    assert(in && src);
    if (src->pos >= src->len) {
        return EOF;
    }

    unsigned char *bytes = src->addr;
    unsigned char next = bytes[src->pos];
    src->pos += 1;

    return next;
}

RBTK_NO_DISCARD size_t
rbtk_read_memory_stream_bytes(RBTK_UNUSED RBTK_IN_STREAM *in,
    rbtk_memory_in_stream_src *src, void *buf, size_t off, size_t len)
{
    assert(in && src && buf);

    size_t cpy_len = len;
    if (src->pos + cpy_len >= src->len) {
        cpy_len = src->len - src->pos;
    }

    unsigned char *buf_bytes = buf;
    memcpy(buf_bytes + off, src->addr, cpy_len);
    memset(buf_bytes + off + cpy_len, 0x00, len - cpy_len);

    return cpy_len;
}

size_t
rbtk_skip_memory_stream_bytes(RBTK_UNUSED RBTK_IN_STREAM *in,
    rbtk_memory_in_stream_src *src, size_t amt)
{
    assert(in && src);

    /* prevent overrun */
    long current_pos = (long) src->pos;
    if (current_pos + amt >= src->len) {
        amt = src->len - current_pos;
    }
    src->pos += amt;

    return amt;
}

size_t
rbtk_seek_memory_stream_to(RBTK_UNUSED RBTK_IN_STREAM *in,
    rbtk_memory_in_stream_src *src, size_t pos)
{
    assert(in && src);
    if (pos >= src->len) {
        src->pos = src->len;
    }
    else {
        src->pos = pos;
    }
    return pos;
}

const rbtk_in_stream_funs rbtk_file_in_stream_funs = {
    .close           = (rbtk_in_stream_close_fun)           rbtk_close_file_in_stream,
    .available_bytes = (rbtk_in_stream_available_bytes_fun) rbtk_available_bytes_in_file_stream,
    .read_byte       = (rbtk_in_stream_read_byte_fun)       rbtk_read_file_stream_byte,
    .read_bytes      = (rbtk_in_stream_read_bytes_fun)      rbtk_read_file_stream_bytes,
    .skip_bytes      = (rbtk_in_stream_skip_bytes_fun)      rbtk_skip_file_stream_bytes,
    .seek_to         = (rbtk_in_stream_seek_to_fun)         rbtk_seek_file_stream_to
};

const rbtk_in_stream_funs rbtk_memory_in_stream_funs = {
    .close           = (rbtk_in_stream_close_fun)           RBTK_NO_OP,
    .available_bytes = (rbtk_in_stream_available_bytes_fun) rbtk_available_bytes_in_memory_stream,
    .read_byte       = (rbtk_in_stream_read_byte_fun)       rbtk_read_memory_stream_byte,
    .read_bytes      = (rbtk_in_stream_read_bytes_fun)      rbtk_read_memory_stream_bytes,
    .skip_bytes      = (rbtk_in_stream_skip_bytes_fun)      rbtk_skip_memory_stream_bytes,
    .seek_to         = (rbtk_in_stream_seek_to_fun)         rbtk_seek_memory_stream_to
};
