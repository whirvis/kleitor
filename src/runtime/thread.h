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
#ifndef RBTK_THREAD_H_
#define RBTK_THREAD_H_

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/*!
 * @file
 * @brief The public API for the program's thread module.
 */

#include <stdbool.h>
#include <stddef.h>

#include "common.h"
#include "time.h"

/*!
 * @defgroup thread Multi-threading
 *
 * @brief The program's thread module.
 * @pre   Initialize the thread module.
 * @post  Terminate the thread module.
 *
 * @see rbtk_current_thread(void)
 * @see rbtk_create_thread(const char *, rbtk_thread_entrypoint, void *)
 *
 * @{
 */

/*!
 * @brief The maximum length of a thread name in bytes, including the `NULL`
 *        terminator.
 *
 * @note This length is arbitrary. Feel free to increase this value if need
 * be. However, ensure it is a power of two (e.g., `64` or `128`).
 */
#define RBTK_THREAD_NAME_MAX_LENGTH 32

/*!
 * @brief A thread of execution in the program.
 *
 * @attention Threads, by default, do not have their own storage. They share
 * their memory with the rest of the program. For thread local storage, call
 * #rbtk_create_thread_storage(size_t size) on the main thread first.
 *
 * @attention Furthermore, resources shared among multiple threads should be
 * treated with caution. Bugs can arise from multiple threads reading and/or
 * writing to the same memory at once.
 *
 * @see rbtk_current_thread(void)
 * @see rbtk_create_thread(const char *, rbtk_thread_entrypoint, void *)
 */
RBTK_FORWARD_DECLARATION
typedef struct RBTK_THREAD RBTK_THREAD;

/*!
 * @brief The entrypoint for a thread.
 *
 * @param[in] args The arguments for startup, may be `NULL`.
 *
 * @see rbtk_create_thread(const char *, rbtk_thread_entrypoint, void *)
 */
typedef void (*rbtk_thread_entrypoint)(void *args);

/*!
 * @brief A key for accessing thread-local storage.
 *
 * @see rbtk_create_thread_storage(size_t)
 */
RBTK_FORWARD_DECLARATION
typedef struct RBTK_THREAD_STORAGE_KEY RBTK_THREAD_STORAGE_KEY;

/*!
 * @brief Describes the priority of a thread.
 *
 * Thread priorities can be used to ensure less important threads do not
 * utilize the same resources more important threads might need.
 *
 * @attention Enforcement of thread priorities are **not** guaranteed.
 * They are only a suggestion to the scheduler of the operating system.
 *
 * @see rbtk_set_thread_priority(RBTK_THREAD *, rbtk_thread_priority)
 */
typedef enum rbtk_thread_priority {
    RBTK_THREAD_PRIORITY_BACKGROUND,   /*!< Run in the background.         */
    RBTK_THREAD_PRIORITY_LOW,          /*!< Operate at low priority.       */
    RBTK_THREAD_PRIORITY_BELOW_NORMAL, /*!< Operate below normal priority. */
    RBTK_THREAD_PRIORITY_NORMAL,       /*!< Operate at normal priority.    */
    RBTK_THREAD_PRIORITY_ABOVE_NORMAL, /*!< Operate above normal priority. */
    RBTK_THREAD_PRIORITY_HIGH,         /*!< Operatoe at high priority.     */
    RBTK_THREAD_PRIORITY_CRITICAL      /*!< Critical for correctness.      */
} rbtk_thread_priority;

/*!
 * @brief Returns the current thread of execution.
 *
 * @return The current thread of execution, `NULL` on failure.
 *
 * @errors
 * @signal{#RBTK_ERROR_ILLEGAL_STATE, If the thread module is not initialized.}
 * @signal{#RBTK_ERROR_PLATFORM,      If a platform specific error occurred.}
 * @enderrors
 *
 * @see RBTK_MAIN_THREAD
 */
RBTK_NO_DISCARD RBTK_THREAD *
rbtk_current_thread(void);

/*!
 * @brief Creates a thread.
 * @post  Optionally, start the thread.
 *
 * @param[in] name       The name of the thread. Any name which exceeds the
 *                       maximum length shall be cut off.
 * @param[in] entrypoint The thread entrypoint function. This function will
 *                       be executed when the thread is started.
 * @param[in] args       The arguments for startup. This can be any type.
 *                       However, it is the caller's responsibility to pass
 *                       the correct data to the thread. This may also be
 *                       `NULL`, assuming `entrypoint` takes no arguments.
 * @return The newly created thread, `NULL` on failure.
 *
 * @pointer_lifetime The returned pointer is valid until the thread is
 * destroyed via #rbtk_destroy_thread(RBTK_THREAD *) or the thread module
 * is terminated. It is an unchecked runtime error to free it.
 *
 * @debugging This function asserts that `name` and `entrypoint` are not
 * `NULL`.
 * @errors
 * @signal{#RBTK_ERROR_ILLEGAL_STATE, If the thread module is not initialized.}
 * @signal{#RBTK_ERROR_PLATFORM,      If a platform specific error occurred.}
 * @enderrors
 *
 * @see rbtk_start_thread(RBTK_THREAD *)
 */
RBTK_NO_DISCARD RBTK_THREAD *
rbtk_create_thread(const char *name, rbtk_thread_entrypoint entrypoint,
    void *args);

/*!
 * @brief Destroys a thread.
 * @pre   Optionally, stop the thread.
 *
 * Destroying a thread will forcefully end execution. This function may
 * also be called for the main thread, which will result in the program
 * terminating.
 *
 * @param[in] thread The thread to destroy, `NULL` for the current thread.
 * @return `true` on success, `false` on failure.
 *
 * @errors
 * @signal{#RBTK_ERROR_PLATFORM, If a platform specific error occurred.}
 * @enderrors
 *
 * @see rbtk_stop_thread(RBTK_THREAD *, bool)
 */
bool
rbtk_destroy_thread(RBTK_THREAD *thread);

/*!
 * @brief Returns the ID of a thread.
 *
 * @param[in] thread The thread to query, `NULL` for the current thread.
 * @return The thread ID.
 */
RBTK_NO_DISCARD size_t
rbtk_get_thread_id(const RBTK_THREAD *thread);

/*!
 * @brief Returns the name of a thread.
 *
 * @param[in] thread The thread to query, `NULL` for the current thread.
 * @return The thread name.
 *
 * @pointer_lifetime The contents of the returned pointer are managed by
 * the thread module. It is an unchecked runtime error to free it.
 */
RBTK_NO_DISCARD const char *
rbtk_get_thread_name(const RBTK_THREAD *thread);

/*!
 * @brief Sets the name of a thread.
 *
 * @param[in] thread The thread to update, `NULL` for the current thread.
 * @param[in] name   The new name of the thread. Any name which exceeds the
 *                   maximum length shall be cut off.
 *
 * @pointer_lifetime The contents of `name` are copied to an internal buffer
 * managed by the thread module.
 *
 * @debugging This function asserts that `name` is not `NULL`.
 */
void
rbtk_set_thread_name(RBTK_THREAD *thread, const char *name);

/*!
 * @brief Returns if a thread is a daemon.
 *
 * @param[in] thread The thread to query, `NULL` for the current thread.
 * @return `true` if the thread is a daemon, `false` otherwise.
 */
RBTK_NO_DISCARD bool
rbtk_thread_is_daemon(RBTK_THREAD *thread);

/*!
 * @brief Sets whether a thread is a daemon or not.
 *
 * The program only waits for non-daemon threads to finish execution before
 * exiting. This makes setting a thread as a daemon useful when it does not
 * perform any critical tasks.
 *
 * @param[in] thread The thread to update, `NULL` for the current thread.
 * @param[in] daemon `true` to make it a daemon, `false` otherwise.
 */
void
rbtk_thread_set_daemon(RBTK_THREAD *thread, bool daemon);

/*!
 * @brief Returns the priority of a thread.
 *
 * @param[in] thread The thread to query, `NULL` for the current thread.
 * @return The thread priority.
 */
RBTK_NO_DISCARD rbtk_thread_priority
rbtk_get_thread_priority(RBTK_THREAD *thread);

/*!
 * @brief Sets the priority of a thread.
 *
 * @param[in] thread   The thread to update, `NULL` for the current thread.
 * @param[in] priority The new priority for the thread.
 * @return `true` if the priority was meaningfully applied to the thread,
 * `false` otherwise.
 *
 * @errors
 * @signal{#RBTK_ERROR_PLATFORM, If a platform specific error occurred.}
 * @enderrors
 */
bool
rbtk_set_thread_priority(RBTK_THREAD *thread, rbtk_thread_priority priority);

/*!
 * @brief Returns if the thread is alive.
 *
 * @param[in] thread The thread to query.
 * @return `true` if the thread is alive, `false` otherwise.
 *
 * @debugging This function asserts that `thread` is not `NULL`.
 *
 * @see rbtk_start_thread(RBTK_THREAD *)
 * @see rbtk_stop_thread(RBTK_THREAD *)
 */
RBTK_NO_DISCARD bool
rbtk_thread_is_alive(RBTK_THREAD *thread);

/*!
 * @brief Starts a thread.
 *
 * @param[in] thread The thread to start.
 * @return `true` on success, `false` on failure.
 *
 * @debugging This function asserts that `thread` is not `NULL`.
 * @errors
 * @signal{#RBTK_ERROR_ILLEGAL_STATE, If the thread is already alive;
 *                                <br>If the thread has already died.}
 * @signal{#RBTK_ERROR_PLATFORM,      If a platform specific error occurred.}
 * @enderrors
 *
 * @see rbtk_thread_is_alive(RBTK_THREAD *)
 */
bool
rbtk_start_thread(RBTK_THREAD *thread);

/*!
 * @brief Stops a thread.
 *
 * @note This function is a no-op if the thread is not alive.
 *
 * @param[in] thread The thread to stop.
 * @return `true` on success, `false` on failure.
 *
 * @debugging This function asserts that `thread` is not `NULL`.
 * @errors
 * @signal{#RBTK_ERROR_PLATFORM, If a platform specific error occurred.}
 * @enderrors
 *
 * @see rbtk_thread_is_alive(RBTK_THREAD *)
 */
bool
rbtk_stop_thread(RBTK_THREAD *thread);

/*!
 * @brief Returns if a thread was interrupted.
 *
 * @param[in] thread     The thread to query, `NULL` for the current thread.
 * @param[in] clear_flag `true` if the interrupt status should be cleared;
 *                       `false` otherwise.
 * @return `true` if the thread was interrupted, `false` otherwise.
 *
 * @see rbtk_interrupt_thread(RBTK_THREAD *)
 */
RBTK_NO_DISCARD bool
rbtk_thread_interrupted(RBTK_THREAD *thread, bool clear_flag);

/*!
 * @brief Interrupts a thread.
 *
 * @warning This function is a no-op if the thread is not alive.
 *
 * @param thread The thread to interrupt, `NULL` for the current thread.
 *
 * @see rbtk_thread_interrupted(RBTK_THREAD *, bool)
 */
void
rbtk_interrupt_thread(RBTK_THREAD *thread);

/*!
 * @brief Sends a signal that a thread is willing to yield.
 *
 * "Yielding" is the process of sending a signal to the processor that
 * a thread is willing to give up its execution time to another thread.
 *
 * @attention Yield signals are only a suggestion to the processor.
 *
 * @param[in] thread The thread to yield, `NULL` for the current thread.
 */
void
rbtk_yield_thread(RBTK_THREAD *thread);

/*!
 * @brief Joins a thread to the calling thread.
 *
 * The act of "joining" results in the calling thread waiting until the
 * thread in question dies. If the thread is not alive, then the function
 * will immediately return.
 *
 * @param[in] thread The thread to join, must not be the calling thread.
 *
 * @debugging This function asserts that `thread` is not `NULL` and not
 * the calling thread and that `timeout` is positive.
 */
void
rbtk_join_thread(RBTK_THREAD *thread);

/*!
 * @brief Joins a thread to the calling thread.
 *
 * The act of "joining" results in the calling thread waiting until the
 * thread in question dies. If the thread is not alive, then the function
 * will immediately return.
 *
 * @param[in] thread  The thread to join, must not be the calling thread.
 * @param[in] unit    How to interpret `timeout`.
 * @param[in] timeout The maximum time to wait before returning.
 * @return `true` if the thread died before this function call timed out,
 * `false` otherwise.
 *
 * @debugging This function asserts that `thread` is not `NULL` and not
 * the calling thread and that `timeout` is positive.
 */
bool
rbtk_join_thread_within(RBTK_THREAD *thread, rbtk_time_unit unit,
    long double timeout);

/*!
 * @brief Creates thread-local storage with the given size.
 *
 * @attention Only the main thread can call this function.
 *
 * @param[in] size The size in bytes.
 * @return The storage key, `NULL` on failure.
 *
 * @pointer_lifetime The returned pointer is valid until the storage is
 * destroyed via #rbtk_destroy_thread_storage(RBTK_THREAD_STORAGE_KEY *)
 * or the thread module is terminated. It is an unchecked runtime error
 * to free it.
 *
 * @debugging This function asserts that `size` is positive.
 * @errors
 * @signal{#RBTK_ERROR_ILLEGAL_STATE, If the thread module is not initialized;
 *                                <br>If the caller is not the main thread.}
 * @signal{#RBTK_ERROR_OUT_OF_MEMORY, On memory allocation failure.}
 * @signal{#RBTK_ERROR_PLATFORM,      If a platform specific error occurred.}
 * @enderrors
 *
 * @see rbtk_destroy_thread_storage(RBTK_THREAD_STORAGE_KEY *)
 */
RBTK_NO_DISCARD RBTK_THREAD_STORAGE_KEY *
rbtk_create_thread_storage(size_t size);

/*!
 * @brief Destroys the storage associated with a given key.
 *
 * @param[in] key The storage key.
 * @return `true` on success, `false` on failure.
 *
 * @debugging This function asserts that `key` is not `NULL`.
 * @errors
 * @signal{#RBTK_ERROR_ILLEGAL_STATE, If the caller is not the main thread.}
 * @signal{#RBTK_ERROR_PLATFORM,      If a platform specific error occurred.}
 * @enderrors
 */
bool
rbtk_destroy_thread_storage(RBTK_THREAD_STORAGE_KEY *key);

/*!
 * @brief Returns the storage associated with a given key.
 *
 * The first time this is called for a thread, the storage associated with
 * the given key shall be allocated and zero initialized.
 *
 * @param[in] key The storage key.
 * @return The storage associated with `key` for the calling thread.
 *
 * @pointer_lifetime The pointer to the data is valid until the storage
 * which manages it is destroyed.
 *
 * @debugging This function asserts that `key` is not `NULL`.
 * @errors
 * @signal{#RBTK_ERROR_OUT_OF_MEMORY, On memory allocation failure.}
 * @signal{#RBTK_ERROR_PLATFORM,      If a platform specific error occurred.}
 * @enderrors
 */
RBTK_NO_DISCARD void *
rbtk_get_thread_storage(RBTK_THREAD_STORAGE_KEY *key);

/*!
 * @brief A pointer to the main thread of this program.
 *
 * @note The thread which initialized the thread module is considered to be
 * the main thread. Furthermore, the main thread has an immutable ID of `0`.
 * It also has the name `"main"` by default.
 *
 * You can check if a current thread is the main thread by writing:
 * @code
 * if (rbtk_current_thread() == RBTK_MAIN_THREAD) {
 *     // ... //
 * }
 * @endcode
 */
extern RBTK_THREAD *const RBTK_MAIN_THREAD;

/*! @} */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* RBTK_THREAD_H_ */
