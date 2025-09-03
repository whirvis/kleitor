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
#ifndef RBTK_ERROR_H_
#define RBTK_ERROR_H_

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/*!
 * @file
 * @brief The public API for the program's error module.
 */

#include <stdbool.h>

#include "common.h"

/*!
 * @defgroup error Error Handling
 *
 * @brief The program's error module.
 * @pre   Initialize the error module.
 * @post  Terminate the error module.
 *
 * @attention This error API should only be used for checked runtime
 * errors. That being, errors not directly caused by the programmer
 * (e.g., passing a `NULL` pointer pointer to a function which forbids
 * it.) The built-in macro `assert(int)` should be used for those.
 *
 * @note All functions which utilize the error system should have a
 * distinct return value that indicates an error has occurred. This
 * prevents the caller from needlessly querying for error info.
 *
 * @see rbtk_signal_error(RBTK_ERROR_CODE, const char *, ...)
 * @see rbtk_get_last_error(const char **)
 * @see rbtk_set_uncaught_error_callback(rbtk_uncaught_error_callback_fun)
 *
 * @{
 */

/*!
 * @brief Syntactic sugar for `int`.
 *
 * This is used to indicate that the integer being returned should be seen
 * as an error code, and not as some other value.
 */
#define RBTK_ERROR_CODE int

/*!
 * @brief The maximum length of an error message in bytes, including the
 * `NULL` terminator.
 *
 * @note This length is arbitrary. Feel free to increase this value if need
 * be. However, ensure it is a power of two (e.g., `8192` or `16384`).
 */
#define RBTK_ERROR_MESSAGE_MAX_LENGTH    4096

/*!
 * @brief No error has occurred in the program.
 */
#define RBTK_ERROR_NONE                  0x00000000

/*!
 * @brief An error has occurred during startup.
 */
#define RBTK_ERROR_STARTUP               0x00000001

/*!
 * @brief An error has occurred during shutdown.
 */
#define RBTK_ERROR_SHUTDOWN              0x00000002

/*!
 * @brief The requested operation is not supported.
 */
#define RBTK_ERROR_UNSUPPORTED           0x00000003

/*!
 * @brief A platform specific error has ocurred.
 */
#define RBTK_ERROR_PLATFORM              0x00000004

/*!
 * @brief A requested memory allocation has failed.
 *
 * The best course of action when encountering this error depends on what
 * was being allocated. For example, if this fails when trying to load an
 * extremely large file, the program can simply fail that operation. But,
 * if this occurs when trying to create the base window, then the program
 * might as well just exit.
 */
#define RBTK_ERROR_OUT_OF_MEMORY         0x00000005

/*!
 * @brief The current thread has been interrupted while blocking.
 *
 * In this case, "blocking" simply means waiting for something to happen
 * before continuing. This can be waiting for user input, for some time to
 * pass, and so on. Thread interruptions are not a big deal and are to be
 * expected when multi-threading. How they are handled depends completely
 * on the thread.
 */
#define RBTK_ERROR_INTERRUPTED           0x00000006

/*!
 * @brief The program has entered an unexpected state.
 *
 * This usually occurs due to a bug in the implementation, not because the
 * user made an error. They are not documented as possible error signals as
 * the user shouldn't be checking for them.
 */
#define RBTK_ERROR_UNEXPECTED_STATE      0x00000007

/*!
 * @brief The program cannot currently perform the operation.
 */
#define RBTK_ERROR_ILLEGAL_STATE         0x00000008

/*!
 * @brief An invalid argument has been passed to an operation.
 */
#define RBTK_ERROR_ILLEGAL_ARGUMENT      0x00000009

/*!
 * @brief An out-of-bounds index has been passed to an operation.
 */
#define RBTK_ERROR_OUT_OF_BOUNDS         0x0000000A

/*!
 * @brief An I/O error has ocurred.
 */
#define RBTK_ERROR_IO                    0x0000000B

/*!
 * @brief A requested file could not be found.
 */
#define RBTK_ERROR_FILE_NOT_FOUND        0x0000000C

/*!
 * @brief The prototype for an uncaught error callback.
 *
 * @param[in] error The error code.
 * @param[in] msg   The error message, may be `NULL`.
 *
 * @pointer_lifetime The memory which `msg` points to is managed by the
 * error module. It is an unchecked runtime error to free it.
 *
 * @see rbtk_set_uncaught_error_callback(rbtk_uncaught_error_callback_fun)
 */
typedef void (*rbtk_uncaught_error_callback_fun)(RBTK_ERROR_CODE error,
    const char *msg);

/*!
 * @brief Sets the callback for uncaught errors on this thread.
 *
 * An error is considered to be uncaught if it is not retrieved before
 * another error is signalled on the current thread. This only sets the
 * callback for uncaught errors on the current thread.
 *
 * @note The callback for each thread is cleared when the error module
 * is terminated.
 *
 * @param[in] callback The callback function, may be `NULL`.
 *
 * @reentrancy This function must not be invoked from a callback.
 */
void
rbtk_set_uncaught_error_callback(rbtk_uncaught_error_callback_fun callback);

/*!
 * @brief Returns the last error on the current thread.
 *
 * The error flag is cleared after calling this method. This should be
 * called only when a function has indicated failure via its return value
 * (e.g., returning `NULL` for an allocation).
 *
 * @note This function will always return #RBTK_ERROR_NONE when the error
 * module is not initialized.
 *
 * @param[out] msg A pointer to write the message to, may be `NULL`.
 * @return The error code.
 *
 * @pointer_lifetime The memory which `msg` points to is managed by the
 * error module. It is an unchecked runtime error to free it.
 *
 * @see rbtk_signal_error(RBTK_ERROR_CODE, const char *, ...)
 */
RBTK_ERROR_CODE
rbtk_get_last_error(const char **msg);

/*! \cond false */
void /* runtime implementation for rbtk_signal_error()   */
rbtk_signal_error_rt(RBTK_ERROR_CODE error,
    const char *error_name, const char *msg, ...);
void /* assertion implementation for rbtk_signal_error() */
rbtk_signal_error_at(RBTK_ERROR_CODE error,
    const char *error_name, const char *msg, ...);
/*! \endcond */

#ifdef NDEBUG
    #define rbtk_signal_error(_error, _msg, ...) rbtk_signal_error_rt((_error), (#_error), (_msg), ##__VA_ARGS__)
#else
    #include <assert.h>
    #include <stdio.h>

    /*
     * Keep this all on the same line so when the assertion fails, the error
     * dialog shows the line the macro was invoked and not a few lines after.
     * Also, use RBTK_ERROR_NONE as the test so the program outputs "Assertion
     * failed: RBTK_ERROR_NONE" (or something in that manner) to the console.
     */
    #define rbtk_signal_error(_error, _msg, ...) rbtk_signal_error_at((_error), (#_error), (_msg), ##__VA_ARGS__); assert(RBTK_ERROR_NONE)
#endif

/*!
 * @def rbtk_signal_error
 * @brief Signals an error on the current thread.
 *
 * This should only be used to signal actual runtime errors, and not to
 * send other messages on the thread. For example, it is not appropriate
 * to signal #RBTK_ERROR_NONE to return a status message.
 *
 * @param[in] _error The error code.
 * @param[in] _msg   The detail message, may be `NULL`. Any error message
 *                   which exceeds the maximum length shall be cut off.
 * @param[in] ...    Format arguments for `msg`, if any.
 *
 * @debugging This function asserts that the error system is initialized
 * and that `_error` is not #RBTK_ERROR_NONE.
 *
 * @pointer_lifetime The contents of `_msg` are copied to an internal buffer
 * managed by the error module.
 *
 * @see rbtk_suggest_error(RBTK_ERROR_CODE error, const char *, ...)
 * @see rbtk_get_last_error(const char **)
 */

/*!
 * @brief Signals an error on the current thread if there isn't one already.
 *
 * This should be used when a signal is expected, but for some reason may not
 * be. For example, one function calling another which then fails. The caller
 * would reasonably expect the module to signal its own error. However, if it
 * did not, this function would ensure one was signalled.
 *
 * @param[in] error The error code.
 * @param[in] msg   The detail message, may be `NULL`. Any error message
 *                  which exceeds the maximum length shall be cut off.
 * @param[in] ...   Format arguments for `msg`, if any.
 * @return `true` if the suggested error was signalled, `false` otherwise.
 *
 * @debugging This function asserts that the error system is initialized
 * and that `error` is not #RBTK_ERROR_NONE.
 *
 * @pointer_lifetime The contents of `msg` are copied to an internal buffer
 * managed by the error module.
 *
 * @see rbtk_signal_error(RBTK_ERROR_CODE error, const char *msg, ...)
 * @see rbtk_get_last_error(const char **)
 */
bool
rbtk_suggest_error(RBTK_ERROR_CODE error, const char *msg, ...);

/*!
 * @brief Aborts the program if an error has been signalled.
 *
 * This can be called after any function which may signal an error. It comes
 * in handy when the only appropriate course of action is to just abort. If
 * an error has occurred, this prints the error code and message (if any) to
 * the standard error stream before exiting the program via `abort(void)`.
 *
 * @note This function is a no-op if the error module is not initialized.
 */
void
rbtk_abort_if_error(void);

/*! @} */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* RBTK_ERROR_H_ */
