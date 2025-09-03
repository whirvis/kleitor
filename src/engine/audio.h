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
#ifndef RBTK_ENGINE_AUDIO_H_
#define RBTK_ENGINE_AUDIO_H_

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/*!
 * @file
 * @brief The public API for the game engine's audio module.
 */

#include <stdbool.h>

#include "../runtime/common.h"
#include "../runtime/stream.h"
#include "../runtime/time.h"

/* from ../libraries/stb_vorbis.h */
RBTK_FORWARD_DECLARATION
typedef struct stb_vorbis stb_vorbis;

/* from ../libraries/minimp3_ex.h */
RBTK_FORWARD_DECLARATION
typedef struct mp3dec_ex_t mp3dec_ex_t;

/*!
 * @defgroup engine_audio Audio System
 * @brief The game engine's audio module.
 *
 * @{
 */

/*!
 * @brief Contains information necessary for audio playback.
 *
 * This data is used by implementations of the audio system to correctly
 * playback the PCM data it is presented.
 *
 * @see rbtk_get_audio_source_info(RBTK_AUDIO_SOURCE *)
 */
typedef struct rbtk_audio_source_info {
    unsigned int frequency_hz;    /*!< The frequency in hertz.         */
    unsigned int channel_count;   /*!< The number of channels.         */
    unsigned int bits_per_sample; /*!< The number of bits in a sample. */
} rbtk_audio_source_info;

/*!
 * @brief Represents an Ogg Vorbis audio source.
 *
 * This data structure is used in the implementation for the Ogg Vorbis
 * audio codec.
 *
 * @see rbtk_source_ogg(RBTK_IN_STREAM *)
 * @see RBTK_SOUND
 */
typedef struct rbtk_vorbis_audio_source {
    RBTK_IN_STREAM *in;     /*!< The input stream being read from.  */

    unsigned char *buffer;  /*!< The temporary data buffer.         */
    size_t buffer_size;     /*!< The length of `buffer` in bytes.   */
    size_t buffer_offset;   /*!< The buffer's current offset.       */
    stb_vorbis *decoder;    /*!< The Ogg Vorbis decoder handle.     */
    size_t header_size;     /*!< The Ogg Vorbis header length.      */

    size_t expected_offset; /*!< The expected read offset in bytes. */

    unsigned int channels;  /*!< The number of channels.            */
    float **outputs;        /*!< The last decoded samples.          */
    size_t outputs_index;   /*!< The index of the last used sample. */
    size_t outputs_size;    /*!< The number of decoded samples.     */
} rbtk_vorbis_audio_source;

/*!
 * @brief Represents an MP3 audio source.
 *
 * This data structure is used in the implementation for the MP3
 * audio codec.
 *
 * @attention This audio source is currently not fully implemented.
 *
 * @see rbtk_source_mp3(RBTK_IN_STREAM *)
 * @see RBTK_SOUND
 */
typedef struct rbtk_mp3_audio_source {
    RBTK_IN_STREAM *in;   /*!< The input stream being read from. */
    mp3dec_ex_t *decoder; /*!< The MP3 decoder handle.           */	
} rbtk_mp3_audio_source;

/*!
 * @brief Represents an audio source.
 *
 * Audio sources are an interface for reading PCM data from anywhere. This
 * includes, but is not limited to: audio files, microphones, sockets, etc.
 * To make things easier, a handful of built-in audio source implementations
 * are provided.
 *
 * @par Built-In Audio Sources
 * - rbtk_source_wav(RBTK_IN_STREAM *)
 * - rbtk_source_ogg(RBTK_IN_STREAM *)
 * - rbtk_source_mp3(RBTK_IN_STREAM *)
 *
 * @see rbtk_source_audio(rbtk_audio_source_funs, void *)
 * @see RBTK_SOUND
 */
typedef struct RBTK_AUDIO_SOURCE RBTK_AUDIO_SOURCE;

/*!
 * @brief Function that closes an audio source.
 *
 * @param[in] src  The audio source to close.
 * @param[in] impl The source's implementation data.
 * @return `true` on success, `false` on failure.
 *
 * @implementation This may be an #RBTK_NO_OP.
 *
 * @debugging Implementors should assert that `src` and `impl` are not
 * `NULL`.
 */
typedef RBTK_ABSTRACT_FUNC bool
(*rbtk_close_audio_source_fun)(RBTK_AUDIO_SOURCE *src, void *impl);

/*!
 * @brief Function that reads PCM data from an audio source.
 *
 * @param[in]  src  The audio source to read from.
 * @param[in]  impl The source's implementation data.
 * @param[in]  off  The read offset in bytes.
 * @param[out] buf  The buffer to read into.
 * @param[in]  len  The number of bytes to attempt to read.
 * @return The actual number of bytes read, `EOF` on end of source, or
 * `INT_MAX` on error.
 *
 * @implementation This must be implemented.
 *
 * @debugging Implementors should assert that `src`, `impl`, and `buf`
 * are not `NULL`.
 */
typedef RBTK_ABSTRACT_FUNC int
(*rbtk_read_pcm_fun)(RBTK_AUDIO_SOURCE *src, void *impl, size_t off,
    unsigned char *buf, size_t len);

/*!
 * @brief Functions for implementing an audio source.
 *
 * @see rbtk_source_audio(rbtk_audio_source_funs, void *)
 */
typedef struct rbtk_audio_source_funs {
    rbtk_close_audio_source_fun close;
    rbtk_read_pcm_fun read_pcm;
} rbtk_audio_source_funs;

/*!
 * @brief A playable sound.
 *
 * No sound is playable without an implementation that specifies how its
 * data should be loaded and played. The two built-in implementations are
 * buffered soudns and streamed soudns. Proper usage of these depends on
 * what they will be playing.
 *
 * @see rbtk_buffer_sound(RBTK_AUDIO_SOURCE *)
 * @see rbtk_stream_sound(RBTK_AUDIO_SOURCE *)
 */
RBTK_FORWARD_DECLARATION
typedef struct RBTK_SOUND RBTK_SOUND;

/*!
 * Represents the current state of a sound.
 */
typedef enum rbtk_sound_state {
    RBTK_SOUND_STATE_STOPPED, /*!< The sound is stopped. */
    RBTK_SOUND_STATE_PLAYING, /*!< The sound is playing. */
    RBTK_SOUND_STATE_PAUSED,  /*!< The sound is paused.  */
} rbtk_sound_state;

/*!
 * @brief Creates an audio source.
 *
 * @param[in] funs The audio source functions.
 * @param[in] info The source's specifications.
 * @param[in] impl The source's implementation data.
 * @return The opened audio source or `NULL` on error.
 *
 * @debugging This function asserts that the functions in `funs` meet
 * their implementation requirements and that `data` is not `NULL`.
 * @errors
 * @signal{#RBTK_ERROR_OUT_OF_MEMORY, On memory allocation failure.}
 * @enderrors
 *
 * @see rbtk_close_audio_source(RBTK_AUDIO_SOURCE *)
 */
RBTK_AUDIO_SOURCE *
rbtk_source_audio(rbtk_audio_source_funs funs,
    rbtk_audio_source_info info, void *impl);

/*!
 * @brief Closes an audio source.
 *
 * @attention If the audio source belongs to an #RBTK_SOUND, the source will
 * automatically be closed by the sound once it is closed. It is a checked
 * runtime error to close a source which is currently being used by a sound.
 *
 * @param[in] src The audio source to close.
 * @return `true` on success, `false` on failure.
 *
 * @debugging This function asserts that `src` is not `NULL`.
 * @errors
 * @signal{#RBTK_ERROR_ILLEGAL_STATE, If `src` is currently in use.}
 * @signal{#RBTK_ERROR_IO,            If an I/O error occurs.}
 * @enderrors
 */
bool
rbtk_close_audio_source(RBTK_AUDIO_SOURCE *src);

/*!
 * @brief Returns an audio source's info.
 *
 * @param[in] src The audio source.
 * @return A pointer to the audio source's info.
 *
 * @pointer_lifetime The returned pointer is valid until the audio source
 * is closed via #rbtk_close_audio_source(RBTK_AUDIO_SOURCE *) or until the
 * audio system is shutdown.
 *
 * @debugging This function asserts that `src` is not `NULL`.
 */
const rbtk_audio_source_info *
rbtk_get_audio_source_info(const RBTK_AUDIO_SOURCE *src);

/*!
 * @brief Reads PCM data from an audio source.
 *
 * @note The amount of PCM data that actually gets read may be less than
 * requested. This could be due to decompression limitations, not enough
 * data being present in the file from the offset, or some other issue.
 *
 * @param[in]  src The audio source to read from.
 * @param[in]  off The read offset in bytes.
 * @param[out] buf The buffer to read into.
 * @param[in]  len The number of bytes to attempt to read.
 * @return The actual number of bytes read, `EOF` on end of source, or
 * `INT_MAX` on error.
 *
 * @debugging This function asserts that `src` and `buf` are not `NULL`.
 * @errors
 * @signal{#RBTK_ERROR_IO, If an I/O error occurs.}
 * @enderrors
 */
RBTK_NO_DISCARD int
rbtk_read_pcm(RBTK_AUDIO_SOURCE *src, size_t off, void *buf, size_t len);

/*!
 * @brief Creates an audio source from a WAV file.
 *
 * @param[in] in The input stream to read from.
 * @return The opened audio source or `NULL` on error.
 *
 * @pointer_lifetime The returned pointer is valid until the audio source
 * is closed via #rbtk_close_audio_source(RBTK_AUDIO_SOURCE *) or until the
 * audio system is shutdown.
 *
 * @debugging This function asserts that `in` is not `NULL`.
 * @errors
 * @signal{#RBTK_ERROR_IO, If an I/O error occurs.}
 * @enderrors
 *
 * @see rbtk_open_file_in_stream(const char *)
 * @see rbtk_close_audio_source(RBTK_AUDIO_SOURCE *)
 */
RBTK_NO_DISCARD RBTK_AUDIO_SOURCE *
rbtk_source_wav(RBTK_IN_STREAM *in);

/*!
 * @brief Creates an audio source from an OGG Vorbis file.
 *
 * @param[in] in The input stream to read from.
 * @return The opened audio source or `NULL` on error.
 *
 * @pointer_lifetime The returned pointer is valid until the audio source
 * is closed via #rbtk_close_audio_source(RBTK_AUDIO_SOURCE *) or until the
 * audio system is shutdown.
 *
 * @debugging This function asserts that `in` is not `NULL`.
 * @errors
 * @signal{#RBTK_ERROR_IO, If an I/O error occurs.}
 * @enderrors
 *
 * @see rbtk_open_file_in_stream(const char *)
 * @see rbtk_close_audio_source(RBTK_AUDIO_SOURCE *)
 */
RBTK_NO_DISCARD RBTK_AUDIO_SOURCE *
rbtk_source_ogg(RBTK_IN_STREAM *in);

/*!
 * @brief Creates an audio source from an MP3 file.
 *
 * @param[in] in The input stream to read from.
 * @return The opened audio source or `NULL` on error.
 *
 * @pointer_lifetime The returned pointer is valid until the audio source
 * is closed via #rbtk_close_audio_source(RBTK_AUDIO_SOURCE *) or until the
 * audio system is shutdown.
 *
 * @debugging This function asserts that `in` is not `NULL`.
 * @errors
 * @signal{#RBTK_ERROR_IO, If an I/O error occurs.}
 * @enderrors
 *
 * @see rbtk_open_file_in_stream(const char *)
 * @see rbtk_close_audio_source(RBTK_AUDIO_SOURCE *)
 */
RBTK_NO_DISCARD RBTK_AUDIO_SOURCE *
rbtk_source_mp3(RBTK_IN_STREAM *in);

/*!
 * @brief Buffers a sound from an audio source.
 *
 * These have all of their audio data buffered into memory at once.
 * Buffered sounds should not be used to play large sound files, such
 * as music. They are intended for smaller audio samples, such as SFX.
 * For larger audio files, streamed sounds are recommended.
 *
 * @attention The created sound will take ownership of `src`. It can
 * not be used with another sound after calling this. Furthermore, it
 * will be closed by the sound when the sound itself is closed.
 *
 * @param[in] src The audio source to read from.
 * @return The buffered sound or `NULL` on error.
 *
 * @pointer_lifetime The returned pointer is valid until the sound is
 * closed via #rbtk_close_sound(RBTK_SOUND *) or until the audio system
 * is shutdown.

 * @debugging This function asserts that `src` is not `NULL`.
 * @errors
 * @signal{#RBTK_ERROR_OUT_OF_MEMORY, On memory allocation failure.}
 * @enderrors
 *
 * @see rbtk_stream_sound(RBTK_AUDIO_SOURCE *)
 * @see rbtk_close_sound(RBTK_SOUND *)
 */
RBTK_NO_DISCARD RBTK_SOUND *
rbtk_buffer_sound(RBTK_AUDIO_SOURCE *src);

/*!
 * @brief Streams a sound from an audio source.
 *
 * These have their audio data buffered into memory as they are playing.
 * Streamed sounds should be used to play small sound files, such as
 * SFX. They are intended for larger audio samples, such as music or
 * narration. For smaller audio files, buffered sounds are recommended.
 *
 * @attention The created sound will take ownership of `src`. It can
 * not be used with another sound after calling this. Furthermore, it
 * will be closed by the sound when the sound itself is closed.
 *
 * @param[in] src The audio source to read from.
 * @return The streamable sound or `NULL` on error.
 *
 * @pointer_lifetime The returned pointer is valid until the sound is
 * closed via #rbtk_close_sound(RBTK_SOUND *) or until the audio system
 * is shutdown.
 *
 * @debugging This function asserts that `src` is not `NULL`.
 * @errors
 * @signal{#RBTK_ERROR_OUT_OF_MEMORY, On memory allocation failure.}
 * @enderrors
 *
 * @see rbtk_buffer_sound(RBTK_AUDIO_SOURCE *)
 * @see rbtk_close_sound(RBTK_SOUND *)
 */
RBTK_NO_DISCARD RBTK_SOUND *
rbtk_stream_sound(RBTK_AUDIO_SOURCE *src);

/*!
 * @brief Closes a sound.
 *
 * Closing a sound results in all resources used to play that sound being
 * freed. As a consequence, a closed sound cannot be re-opened and cannot
 * be played. Sounds which are playing will automatically be stopped when
 * they are closed.
 *
 * @param[in] sound The sound to close.
 *
 * @debugging This function asserts that `sound` is not `NULL`.
 */
void
rbtk_close_sound(RBTK_SOUND *sound);

/*!
 * @brief Returns the current state of a sound.
 *
 * @param[in] sound The sound to query.
 * @return The current state of `sound`.
 *
 * @debugging This function asserts that `sound` is not `NULL`.
 *
 * @see rbtk_play_sound(RBTK_SOUND *)
 * @see rbtk_pause_sound(RBTK_SOUND *)
 * @see rbtk_stop_sound(RBTK_SOUND *)
 */
RBTK_NO_DISCARD rbtk_sound_state
rbtk_get_sound_state(const RBTK_SOUND *sound);

/*!
 * @brief Sets the volume of a sound.
 *
 * @param[in] sound  The sound to update.
 * @param[in] volume The new volume to use. The argument for this
 *                   parameter is capped between `0.0f` and `1.0f`.
 *
 * @debugging This function asserts that `sound` is not `NULL`.
 *
 * @see rbtk_increase_volume(RBTK_SOUND *, float)
 * @see rbtk_decrease_volume(RBTK_SOUND *, float)
 */
void
rbtk_set_sound_volume(RBTK_SOUND *sound, float volume);

/*!
 * @brief Increases the volume of a sound.
 *
 * @param[in] sound  The sound to update.
 * @param[in] amount The amount to increase the volume by.
 *
 * @debugging This function asserts that `sound` is not `NULL`.
 *
 * @see rbtk_set_sound_volume(RBTK_SOUND *, float)
 */
void
rbtk_increase_volume(RBTK_SOUND *sound, float amount);

/*!
 * @brief Decreases the volume of a sound.
 *
 * @param[in] sound  The sound to update.
 * @param[in] amount The amount to decrease the volume by.
 *
 * @debugging This function asserts that `sound` is not `NULL`.
 *
 * @see rbtk_set_sound_volume(RBTK_SOUND *, float)
 */
void
rbtk_decrease_volume(RBTK_SOUND *sound, float amount);

/*!
 * @brief Returns the volume of a sound.
 *
 * @param[in] sound The sound to query.
 * @return The current volume of the sound.
 *
 * @debugging This function asserts that `sound` is not `NULL`.
 */
float
rbtk_get_sound_volume(const RBTK_SOUND *sound);


/*!
 * @brief Plays a sound.
 *
 * Where the sound starts playing depends on whether it is currntly stopped
 * or paused. A sound which is stopped (or has not been played) will start
 * from the beginning. A sound which is paused will continue from where it
 * left off.
 *
 * @param[in] sound The sound to play.
 *
 * @debugging This function asserts that `sound` is not `NULL`.
 *
 * @see rbtk_get_sound_state(const RBTK_SOUND *)
 */
void
rbtk_play_sound(RBTK_SOUND *sound);

/*!
 * @brief Pauses a sound.
 *
 * @note Pausing a sound is different from stopping it. If the sound is
 * played after being paused, it will continue from where it left off.
 *
 * @param[in] sound The sound to pause.
 *
 * @debugging This function asserts that `sound` is not `NULL`.
 *
 * @see rbtk_get_sound_state(const RBTK_SOUND *)
 * @see rbtk_stop_sound(RBTK_SOUND *)
 */
void
rbtk_pause_sound(RBTK_SOUND *sound);

/*!
 * @brief Stops a sound.
 *
 * @note Stopping a sound is different from pausing it. If the sound is
 * played after being stopped, it will start from the beginning.
 *
 * @param[in] sound The sound to stop.
 *
 * @debugging This function asserts that `sound` is not `NULL`.
 *
 * @see rbtk_get_sound_state(const RBTK_SOUND *)
 * @see rbtk_pause_sound(RBTK_SOUND *)
 */
void
rbtk_stop_sound(RBTK_SOUND *sound);

/*!
 * @brief Returns if a sound is looping.
 *
 * @note Looping does not indicate if the sound is currently playing.
 * A return value of `true` only means the sound will loop when it is
 * playing.
 *
 * @param[in] sound The sound to query.
 * @return `true` if the sound is looping, `false` otherwise.
 *
 * @debugging This function asserts that `sound` is not `NULL`.
 */
RBTK_NO_DISCARD bool
rbtk_sound_is_looping(const RBTK_SOUND *sound);

/*!
 * @brief Sets whether a sound should loop.
 *
 * @note Setting a sound to loop or not will not cause it to start or stop
 * playing. When looping, a sound automatically goes back to the beginning
 * (instead of stopping) whenever it reaches the end.
 *
 * @param[in] sound The sound to update.
 * @param[in] loop  `true` if the sound should loop, `false` otherwise.
 *
 * @debugging This function asserts that `sound` is not `NULL`.
 */
void
rbtk_loop_sound(RBTK_SOUND *sound, bool loop);

/*!
 * @brief Returns the offset of a sound.
 *
 * @param[in] sound The sound to query.
 * @param[in] unit  How to interpret the offset.
 * @return The offset of the sound.
 *
 * @debugging This function asserts that `sound` is not `NULL`.
 */
RBTK_NO_DISCARD long double
rbtk_get_sound_offset(const RBTK_SOUND *sound, rbtk_time_unit unit);

/*!
 * @brief Seeks a sound to the given offset.
 *
 * @param[in] sound  The sound to update.
 * @param[in] unit   How to interpret `offset`.
 * @param[in] offset The desired offset.
 *
 * @debugging This function asserts that `sound` is not `NULL` and that
 * `offset` is not negative.
 *
 * @see rbtk_skip_sound(RBTK_SOUND *, rbtk_time_unit, long double)
 */
void
rbtk_set_sound_offset(RBTK_SOUND *sound, rbtk_time_unit unit, long double offset);

/*!
 * @brief Skips a sound by the given amount.
 *
 * @param[in] sound  The sound to update.
 * @param[in] unit   How to interpret `offset`.
 * @param[in] amount How much to skip. A negative value is permitted, and
 *                   will result in the sound rewinding instead.
 *
 * @debugging This function asserts that `sound` is not `NULL`.
 *
 * @see rbtk_set_sound_offset(RBTK_SOUND *, rbtk_time_unit, long double)
 */
void
rbtk_skip_sound(RBTK_SOUND *sound, rbtk_time_unit unit, long double amount);

/*! @} */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* RBTK_ENGINE_AUDIO_H_ */
