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
#ifndef RBTK_STREAM_H_
#define RBTK_STREAM_H_

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/*!
 * @file
 * @brief The public API for the program's streaming module.
 */

#include <stdbool.h>
#include <stdio.h>

#include "common.h"

/*!
 * @defgroup stream I/O Streams
 *
 * @brief The program's streaming module.
 *
 * This module exists as the stand library's built-in `FILE` stream is not
 * very flexible. While functions like `fopencookie()` exist, they are not
 * part of the standard library. As such, this module provides types which
 * have the flexibility of custom streams while being cross compatible.
 *
 * A number of utility functions are also offered, with the goal of making
 * reading from and writing to streams easier. These functions also account
 * for endianness. As such, the caller need only specify the endianness of
 * input or the desired endianness of output.
 *
 * This module is inspired by [Java's `io` package.](https://docs.oracle.com\
 * /javase/8/docs/api/java/io/package-summary.html)
 *
 * @see RBTK_IN_STREAM
 *
 * @{
 */

/*!
 * @brief Container for a value used in I/O operations.
 */
typedef union rbtk_io_value {
    union {
	int_least8_t   i8;
	uint_least8_t  u8;
	int_least16_t  i16;
	uint_least16_t u16;
	int_least32_t  i24;
	uint_least32_t u24;
	int_least32_t  i32;
	uint_least32_t u32;
	int_least64_t  i64;
	uint_least64_t u64;
    };
    union {
	float  f32;
	double f64;
    };
} rbtk_io_value;

/*!
 * @brief Wrapper for #rbtk_io_value which contains an error flag.
 *
 * @see rbtk_io_read_be(RBTK_IN_STREAM *, size_t)
 * @see rbtk_io_read_le(RBTK_IN_STREAM *, size_t)
 */
typedef struct rbtk_io_read_result {
    bool error;          /*!< `true` on I/O error, `false` otherwise. */
    rbtk_io_value value; /*!< The read value.                         */
} rbtk_io_read_result;

/*!
 * @brief Represents an input stream.
 *
 * Input streams provide an interface for reading data from a variety of
 * different sources.
 *
 * While all streams must have the ability to read data, not all are made
 * the same. For example: while one stream may support seeking (e.g., file
 * streams), others (e.g., socket streams) will not. This is usually due
 * to limits imposed on implementors by the nature of the data source.
 *
 * @note All streams must return data in *unsigned bytes* (`unsigned char`).
 *
 * @see rbtk_open_in_stream(rbtk_in_stream_funs, void *)
 */
RBTK_FORWARD_DECLARATION
typedef struct RBTK_IN_STREAM RBTK_IN_STREAM;

/*!
 * @brief Function that closes an input stream.
 *
 * @param[in] in  The input stream.
 * @param[in] src The stream's data source.
 * @return `true` on success, `false` on failure.
 *
 * @implementation This may be an #RBTK_NO_OP.
 *
 * @debugging Implementors should assert that `in` and `src` are not `NULL`.
 *
 * @see rbtk_close_in_stream(RBTK_IN_STREAM *)
 */
typedef RBTK_ABSTRACT_FUNC RBTK_NO_DISCARD bool
(*rbtk_in_stream_close_fun)(RBTK_IN_STREAM *in, void *src);

/*!
 * @brief Function that returns how many bytes are left in an input stream.
 *
 * The returned number of bytes may be an estimate, rather than an exact
 * number. This should be left unimplemented if the number of bytes cannot
 * be reasonably determined.
 *
 * @param[in] in  The input stream.
 * @param[in] src The stream's data source.
 * @return The number of bytes remaining or `SIZE_MAX` on error.
 *
 * @implementation This may be #RBTK_UNIMPLEMENTED.
 *
 * @debugging Implementors should assert that `in` and `src` are not `NULL`.
 *
 * @see rbtk_available_bytes(RBTK_IN_STREAM *)
 */
typedef RBTK_ABSTRACT_FUNC RBTK_NO_DISCARD size_t
(*rbtk_in_stream_available_bytes_fun)(RBTK_IN_STREAM *in, void *src);

/*!
 * @brief Function that reads a single byte from an input stream.
 *
 * @param[in] in  The input stream.
 * @param[in] src The stream's data source.
 * @return The read byte, `EOF` on end of stream, or `SHRT_MAX` on error.
 *
 * @implementation This must be implemented.
 *
 * @debugging Implementors should assert that `in` and `src` are not `NULL`.
 *
 * @see rbtk_read_byte(RBTK_IN_STREAM *)
 */
typedef RBTK_ABSTRACT_FUNC RBTK_NO_DISCARD short
(*rbtk_in_stream_read_byte_fun)(RBTK_IN_STREAM *in, void *src);

/*!
 * @brief Function that reads a number of bytes from an input stream.
 *
 * @param[in]  in  The input stream.
 * @param[in]  src The stream's data source.
 * @param[out] buf The buffer to read to.
 * @param[in]  off The offset to begin writing at.
 * @param[in]  len The number of bytes to read.
 * @return The number of bytes actually read.
 *
 * @implementation This may be #RBTK_DEFAULT_IMPL.
 * <p>
 * The default implementation uses #rbtk_read_byte(RBTK_IN_STREAM *in)
 * to read each byte. Implementors are encouraged to implement this if
 * it will achieve faster read times than the default.
 *
 * @debugging Implementors should assert that `in`, `src`, and `buf`
 * are not `NULL`.
 *
 * @see rbtk_read_bytes(RBTK_IN_STREAM *, void *, size_t, size_t)
 */
typedef RBTK_DEFAULT_FUNC RBTK_NO_DISCARD size_t
(*rbtk_in_stream_read_bytes_fun)(RBTK_IN_STREAM* in, void* src, void *buf,
    size_t off, size_t len);

/*!
 * @brief Function that skips over a number of bytes in a stream.
 *
 * @param[in] in  The input stream.
 * @param[in] src The stream's data source.
 * @param[in] amt The number of bytes to skip.
 * @return The number of bytes actually skipped or `SIZE_MAX` on error.
 *
 * @implementation This may be #RBTK_DEFAULT_IMPL.
 * <p>
 * The default implementation uses #rbtk_read_byte(RBTK_IN_STREAM *in)
 * to skip over each byte. Implementors are encouraged to implement this
 * if it will achieve faster read times than the default.
 *
 * @debugging Implementors should assert that `in` and `src` are not
 * `NULL`.
 *
 * @see rbtk_skip_bytes(RBTK_IN_STREAM *, size_t)
 */
typedef RBTK_DEFAULT_FUNC size_t
(*rbtk_in_stream_skip_bytes_fun)(RBTK_IN_STREAM *in, void *src, size_t amt);

/*!
 * @brief Function that seeks an input stream to the given location.
 *
 * @param[in] in  The input stream.
 * @param[in] src The stream's data source.
 * @param[in] pos The position (in bytes).
 * @return `true` on success, `false` on failure.
 *
 * @implementation This may be #RBTK_UNIMPLEMENTED.
 *
 * @debugging Implementors should assert that `in` and `src` are not
 * `NULL`.
 *
 * @see rbtk_seek_to(RBTK_IN_STREAM *, size_t)
 */
typedef RBTK_ABSTRACT_FUNC size_t
(*rbtk_in_stream_seek_to_fun)(RBTK_IN_STREAM *in, void *src, size_t pos);

/*!
 * @brief Functions for implementing an input stream.
 *
 * @see rbtk_open_stream(rbtk_in_stream_funs, void *)
 */
typedef struct rbtk_in_stream_funs {
    rbtk_in_stream_close_fun           close;
    rbtk_in_stream_available_bytes_fun available_bytes;
    rbtk_in_stream_read_byte_fun       read_byte;
    rbtk_in_stream_read_bytes_fun      read_bytes;
    rbtk_in_stream_skip_bytes_fun      skip_bytes;
    rbtk_in_stream_seek_to_fun         seek_to;
} rbtk_in_stream_funs;

/*!
 * @brief Source type for a file input stream.
 *
 * @see rbtk_open_file_in_stream(const char *)
 */
typedef struct rbtk_file_in_stream_src {
    FILE *file;
    size_t size;
} rbtk_file_in_stream_src;

/*!
 * @brief Source type for a memory input stream.
 *
 * @see rbtk_open_memory_in_stream(void *)
 */
typedef struct rbtk_memory_in_stream_src {
    void *addr;
    size_t len;
    size_t pos;
} rbtk_memory_in_stream_src;

/*!
 * @brief Opens an input stream.
 *
 * @param[in] funs The input stream functions.
 * @param[in] src  The input stream source data.
 * @return The opened input stream or `NULL` on error.
 *
 * @debugging This function asserts that the functions in `funs` meet
 * their implementation requirements and that `src` is not `NULL`.
 * @errors
 * @signal{#RBTK_ERROR_OUT_OF_MEMORY, On memory allocation failure.}
 * @enderrors
 *
 * @see rbtk_close_in_stream(RBTK_IN_STREAM *)
 */
RBTK_NO_DISCARD RBTK_IN_STREAM *
rbtk_open_in_stream(rbtk_in_stream_funs funs, void *src);

/*!
 * @brief Closes an input stream.
 *
 * @param[in] in The input stream.
 * @return `true` on success, `false` on failure.
 *
 * @debugging This function asserts that `in` is not `NULL`.
 * @errors
 * @signal{#RBTK_ERROR_IO, If an I/O error occurs.}
 * @enderrors
 */
bool
rbtk_close_in_stream(RBTK_IN_STREAM *in);

/*!
 * @brief Returns if an input stream can see how many bytes remain.
 *
 * @param[in] in The input stream.
 * @return `true` if the stream can see how many bytes remain, `false`
 * otherwise.
 *
 * @debugging This function asserts that `in` is not `NULL`.
 *
 * @see rbtk_available_bytes(RBTK_IN_STREAM *, size_t)
 */
RBTK_NO_DISCARD bool
rbtk_supports_available_bytes(RBTK_IN_STREAM *in);

/*!
 * @brief Returns how many bytes are left in an input stream.
 *
 * @note The returned number of bytes may only be an estimate, rather than
 * an exact number. As such, it should not be used to determine how large a
 * buffer should be to hold the contents of said stream.
 *
 * @param[in] in The input stream.
 * @return The number of bytes remaining or `SIZE_MAX` on error.
 *
 * @debugging This function asserts that `in` is not `NULL`.
 * @errors
 * @signal{#RBTK_ERROR_IO, If an I/O error occurs.}
 * @enderrors
 *
 * @see rbtk_supports_available_bytes(RBTK_IN_STREAM *)
 */
RBTK_NO_DISCARD size_t
rbtk_available_bytes(RBTK_IN_STREAM *in);

/*!
 * @brief Reads a single byte from an input stream.
 *
 * @note The return value type is deliberately `signed short` so values
 * other than `0x00` to `0xFF` can be returned. That being, `EOF` for end
 * of stream. Any other return value indicates an error.
 *
 * @param[in] in The input stream.
 * @return The read byte, `EOF` on end of stream, or `SHRT_MAX` on error.
 *
 * @debugging This function asserts that `in` is not `NULL`.
 * @errors
 * @signal{#RBTK_ERROR_IO, If an I/O error occurs.}
 * @enderrors
 *
 * @see rbtk_read_bytes(RBTK_IN_STREAM *, void *, size_t, size_t)
 */
RBTK_NO_DISCARD short
rbtk_read_byte(RBTK_IN_STREAM *in);

/*!
 * @brief Reads multiple bytes from an input stream into a buffer.
 *
 * @attention This function may not read the requested number of bytes.
 * This is usually caused by reaching the end of stream. When this occurs,
 * remaining bytes are zero initialized to prevent uninitialized data.
 *
 * @param[in]  in  The input stream.
 * @param[out] buf The buffer to read bytes into.
 * @param[in]  off The offset to start writing at.
 * @param[in]  len The number of bytes to read.
 * @return The number of bytes actually read or `SIZE_MAX` on error.
 *
 * @debugging This function asserts that `in` and `buf` are not `NULL`.
 * @errors
 * @signal{#RBTK_ERROR_IO, If an I/O error occurs.}
 * @enderrors
 *
 * @see rbtk_read_byte(RBTK_IN_STREAM *)
 */
RBTK_NO_DISCARD size_t
rbtk_read_bytes(RBTK_IN_STREAM *in, void *buf, size_t off, size_t len);

/*!
 * @brief Reads the remaining contents of a stream into a single buffer.
 *
 * @param[in]  in   The input stream.
 * @param[out] size The final size of the buffer.
 * @return The remaining contents of the stream.
 *
 * @debugging This function asserts that `in` and `buf` are not `NULL`.
 * @errors
 * @signal{#RBTK_ERROR_IO, If an I/O error occurs.}
 * @enderrors
 *
 * @see rbtk_read_bytes(RBTK_IN_STREAM *, void *, size_t, size_t)
 */
RBTK_NO_DISCARD void *
rbtk_buffer_remaining(RBTK_IN_STREAM *in, size_t *size);

/*!
 * @brief Skips over a number of bytes in an input stream.
 *
 * @note This function may not skip the requested number of bytes. This is
 * usually caused by reaching the end of the stream. When this occurs, the
 * function just returns early.
 *
 * @param[in] in  The input stream.
 * @param[in] amt The number of bytes to skip.
 * @return The number of bytes actually skipped or `SIZE_MAX` on error.
 *
 * @debugging This function asserts that `in` is not `NULL`.
 * @errors
 * @signal{#RBTK_ERROR_IO, If an I/O error occurs.}
 * @enderrors
 */
size_t
rbtk_skip_bytes(RBTK_IN_STREAM *in, size_t amt);

/*!
 * @brief Returns if an input stream supports seeking
 *
 * @param[in] in The input stream.
 * @return `true` if the stream supports seeking, `false` otherwise.
 *
 * @debugging This function asserts that `in` is not `NULL`.
 *
 * @see rbtk_seek_to(RBTK_IN_STREAM *, size_t)
 */
RBTK_NO_DISCARD bool
rbtk_supports_seek(RBTK_IN_STREAM *in);

/*!
 * @brief Seeks an input stream to the given position.
 *
 * @param[in] in  The input stream.
 * @param[in] pos The position, in bytes, to seek to.
 * @return The new position or `SIZE_MAX` on error.
 *
 * @debugging This function asserts that `in` is not `NULL`.
 * @errors
 * @signal{#RBTK_ERROR_IO, If an I/O error occurs.}
 * @enderrors
 *
 * @see rbtk_supports_seek(RBTK_IN_STREAM *)
 */
size_t
rbtk_seek_to(RBTK_IN_STREAM *in, size_t pos);

/*!
 * @brief Reads a big-endian value from an input stream.
 *
 * The current machine's endianness is accounted for when reading the
 * value. If necessary, the bytes will be swapped so they are correctly
 * interpreted by the machine.
 *
 * @return the result of reading. On error, the `size` field of the
 * result shall be `SIZE_MAX`.
 *
 * @debugging This function asserts that `in` is not `NULL` and `size`
 * is not greater than `sizeof(rbtk_io_read_value)`.
 * @errors
 * @signal{#RBTK_ERROR_IO, If an I/O error occurs.}
 * @enderrors
 *
 * @see rbtk_io_read_le(RBTK_IN_STREAM *, size_t)
 */
rbtk_io_read_result
rbtk_io_read_be(RBTK_IN_STREAM* in, size_t type);

/*!
 * @brief Reads a little-endian value from an input stream.
 *
 * The current machine's endianness is accounted for when reading the
 * value. If necessary, the bytes will be swapped so they are correctly
 * interpreted by the machine.
 *
 * @return the result of reading. On error, the `type` field of the
 * result shall be set to `RBTK_IO_INVALID`.
 *
 * @debugging This function asserts that `in` is not `NULL` and `size`
 * is not greater than `sizeof(rbtk_io_read_value)`.
 * @errors
 * @signal{#RBTK_ERROR_IO, If an I/O error occurs.}
 * @enderrors
 *
 * @see rbtk_io_read_be(RBTK_IN_STREAM *, size_t)
 */
rbtk_io_read_result
rbtk_io_read_le(RBTK_IN_STREAM* in, size_t size);

/*!
 * @brief Opens a file input stream.
 *
 * The file is opened using `fopen()` from the C standard library, and
 * is opened in `rb` (read binary) mode. This file will be closed with
 * `fclose()` when the stream is closed.
 *
 * @param[in] filepath The path of the file to open.
 * @return The opened stream, `NULL` on error.
 *
 * @debugging This function asserts that `filepath` is not `NULL`.
 * @errors
 * @signal{#RBTK_ERROR_IO, If an I/O error occurs.}
 * @enderrors
 *
 * @see rbtk_open_in_stream(rbtk_in_stream_funs, void *)
 */
RBTK_NO_DISCARD RBTK_IN_STREAM *
rbtk_open_file_in_stream(const char *filepath);

/*!
 * @brief Closes a file input stream.
 *
 * @param[in] in  The file input stream.
 * @param[in] src The stream's source file.
 * @return `true` on success, `false` on failure.
 *
 * @debugging This function asserts that `in` and `src` are not `NULL`.
 * @errors
 * @signal{#RBTK_ERROR_IO, If an I/O error occurs.}
 * @enderrors
 *
 * @see rbtk_close_in_stream(RBTK_IN_STREAM *)
 */
bool
rbtk_close_file_in_stream(RBTK_IN_STREAM *in,
    rbtk_file_in_stream_src *src);

/*!
 * @brief Returns how many bytes are left in a file input stream.
 *
 * @param[in] in  The file input stream.
 * @param[in] src The stream's source file.
 * @return The number of bytes remaining, `SIZE_MAX` on error.
 *
 * @debugging This function asserts that `in` and `src` are not `NULL`.
 * @errors
 * @signal{#RBTK_ERROR_IO, If an I/O error occurs.}
 * @enderrors
 *
 * @see rbtk_available_bytes(RBTK_IN_STREAM *)
 */
RBTK_NO_DISCARD size_t
rbtk_available_bytes_in_file_stream(RBTK_IN_STREAM *in,
    rbtk_file_in_stream_src *src);

/*!
 * @brief Reads a single byte from a file input stream.
 *
 * @param[in] in  The file input stream.
 * @param[in] src The stream's source file.
 * @return The read byte, `EOF` on end of stream, or `SHRT_MAX` on error.
 *
 * @debugging This function asserts that `in` and `src` are not `NULL`.
 * @errors
 * @signal{#RBTK_ERROR_IO, If an I/O error occurs.}
 * @enderrors
 *
 * @see rbtk_read_byte(RBTK_IN_STREAM *)
 */
RBTK_NO_DISCARD short
rbtk_read_file_stream_byte(RBTK_IN_STREAM *in,
    rbtk_file_in_stream_src *src);

/*!
 * @brief Reads multiple bytes from a file input stream into a buffer.
 *
 * @attention This function may not read the requested number of bytes.
 * This is usually caused by reaching the end of stream. When this occurs,
 * remaining bytes are zero initialized to prevent uninitialized data.
 *
 * @param[in]  in  The file input stream.
 * @param[in]  src The stream's source file.
 * @param[out] buf The buffer to read bytes into.
 * @param[in]  off The offset to start writing at.
 * @param[in]  len The number of bytes to read.
 * @return The number of bytes actually read or `SIZE_MAX` on error.
 *
 * @debugging This function asserts that `in`, `src` and `buf` are not
 * `NULL`.
 * @errors
 * @signal{#RBTK_ERROR_IO, If an I/O error occurs.}
 * @enderrors
 *
 * @see rbtk_read_bytes(RBTK_IN_STREAM *, void *, size_t, size_t)
 */
RBTK_NO_DISCARD size_t
rbtk_read_file_stream_bytes(RBTK_IN_STREAM *in,
    rbtk_file_in_stream_src *src, void *buf, size_t off, size_t len);

/*!
 * @brief Skips over a number of bytes in a file input stream.
 *
 * @param[in] in  The file input stream.
 * @param[in] src The stream's source file.
 * @param[in] amt The number of bytes to skip.
 * @return The number of bytes actually skipped or `SIZE_MAX` on error.
 *
 * @debugging This function asserts that `in` and `src` are not `NULL`.
 * @errors
 * @signal{#RBTK_ERROR_IO, If an I/O error occurs.}
 * @enderrors
 *
 * @see rbtk_skip_bytes(RBTK_IN_STREAM *, size_t)
 */
size_t
rbtk_skip_file_stream_bytes(RBTK_IN_STREAM *in,
    rbtk_file_in_stream_src *src, size_t amt);

/*!
 * @brief Seeks a file input stream to the given position.
 *
 * @param[in] in  The file input stream.
 * @param[in] src The stream's source file.
 * @param[in] pos The position, in bytes, to seek to.
 * @return The new position or `SIZE_MAX` on error.
 *
 * @debugging This function asserts that `in` and `src` are not `NULL`.
 * @errors
 * @signal{#RBTK_ERROR_IO, If an I/O error occurs.}
 * @enderrors
 *
 * @see rbtk_seek_to(RBTK_IN_STREAM *, size_t)
 */
size_t
rbtk_seek_file_stream_to(RBTK_IN_STREAM *in,
    rbtk_file_in_stream_src *src, size_t pos);

/*!
 * @brief Opens a memory input stream.
 *
 * @param[in] addr The memory address to start reading from.
 * @param[in] len  The number of bytes available at the memory address.
 * @return The opened stream.
 *
 * @debugging This function asserts that `addr` is not `NULL`.
 *
 * @see rbtk_open_in_stream(rbtk_in_stream_funs, void *)
 */
RBTK_NO_DISCARD RBTK_IN_STREAM *
rbtk_open_memory_in_stream(void *addr, size_t len);

/*!
 * @brief Returns how many bytes are left in a memory input stream.
 *
 * @param[in] in  The memory input stream.
 * @param[in] src The stream's source address.
 * @return The number of bytes remaining.
 *
 * @debugging This function asserts that `in` and `src` are not `NULL`.
 *
 * @see rbtk_available_bytes(RBTK_IN_STREAM *)
 */
RBTK_NO_DISCARD size_t
rbtk_available_bytes_in_memory_stream(RBTK_IN_STREAM *in,
    rbtk_memory_in_stream_src *src);

/*!
 * @brief Reads a single byte from a memory input stream.
 *
 * @param[in] in  The memory input stream.
 * @param[in] src The stream's source address.
 * @return The read byte or `EOF` on end of stream.
 *
 * @debugging This function asserts that `in` and `src` are not `NULL`.
 *
 * @see rbtk_read_byte(RBTK_IN_STREAM *)
 */
RBTK_NO_DISCARD short
rbtk_read_memory_stream_byte(RBTK_IN_STREAM *in,
    rbtk_memory_in_stream_src *src);

/*!
 * @brief Reads multiple bytes from a memory input stream into a buffer.
 *
 * @attention This function may not read the requested number of bytes.
 * This is usually caused by reaching the end of stream. When this occurs,
 * remaining bytes are zero initialized to prevent uninitialized data.
 *
 * @param[in]  in  The memory input stream.
 * @param[in]  src The stream's source address.
 * @param[out] buf The buffer to read bytes into.
 * @param[in]  off The offset to start writing at.
 * @param[in]  len The number of bytes to read.
 * @return The number of bytes actually read.
 *
 * @debugging This function asserts that `in`, `src` and `buf` are not
 * `NULL`.
 *
 * @see rbtk_read_bytes(RBTK_IN_STREAM *, void *, size_t, size_t)
 */
RBTK_NO_DISCARD size_t
rbtk_read_memory_stream_bytes(RBTK_IN_STREAM *in,
    rbtk_memory_in_stream_src *src, void *buf, size_t off, size_t len);

/*!
 * @brief Skips over a number of bytes in a memory input stream.
 *
 * @param[in] in  The memory input stream.
 * @param[in] src The stream's source address.
 * @param[in] amt The number of bytes to skip.
 * @return The number of bytes actually skipped.
 *
 * @debugging This function asserts that `in` and `src` are not `NULL`.
 *
 * @see rbtk_skip_bytes(RBTK_IN_STREAM *, size_t)
 */
size_t
rbtk_skip_memory_stream_bytes(RBTK_IN_STREAM *in,
    rbtk_memory_in_stream_src *src, size_t amt);

/*!
 * @brief Seeks a memory input stream to the given position.
 *
 * @param[in] in  The memory input stream.
 * @param[in] src The stream's source address.
 * @param[in] pos The position, in bytes, to seek to.
 * @return The new position.
 *
 * @debugging This function asserts that `in` and `src` are not `NULL`.
 *
 * @see rbtk_seek_to(RBTK_IN_STREAM *, size_t)
 */
size_t
rbtk_seek_memory_stream_to(RBTK_IN_STREAM *in,
    rbtk_memory_in_stream_src *src, size_t pos);

/*!
 * @brief The functions which implement a file input stream.
 *
 * @see rbtk_open_file_in_stream(const char *)
 */
extern const rbtk_in_stream_funs rbtk_file_in_stream_funs;

/*!
 * @brief The functions which implement a memory input stream.
 *
 * @see rbtk_open_memory_in_stream(void *)
 */
extern const rbtk_in_stream_funs rbtk_memory_in_stream_funs;

/*! @} */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* RBTK_STREAM_H_ */
