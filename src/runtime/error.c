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
#include "error.h"
#include "./private/error.h"
#include "./platform/error.h"

#include <assert.h>
#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"

static const char *truncate_msg = "[! error message truncated !]";

static size_t truncate_msg_len;
static bool initialized;

static priv_rbtk_error_info *
get_error_info(bool allocate_if_missing)
{
    bool allocated = false;
    RBTK_ERROR_CODE error = RBTK_ERROR_NONE;
    priv_rbtk_error_info *info = plat_rbtk_get_error_info(
        allocate_if_missing, &allocated, &error);

    if (error) {
        fprintf(stderr, "Failed to allocate error info: %d\n", error);
        abort(); /* we cannot recover from this */
    }

    if (allocated) {
        info->signaled = false;
        info->error = RBTK_ERROR_NONE;
        info->msg[0] = '\0';
        info->uncaught_error_fun = (rbtk_uncaught_error_callback_fun) RBTK_NO_OP;
    }

    return info;
}

RBTK_NO_DISCARD bool
priv_rbtk_error_init(void)
{
    if (initialized) {
        return true;
    }

    if (!plat_rbtk_error_init()) {
        return false;
    }

    truncate_msg_len = strlen(truncate_msg);

    initialized = true;
    return true;
}

RBTK_NO_DISCARD bool
priv_rbtk_error_terminate(void)
{
    if (!initialized) {
        return true;
    }

    priv_rbtk_error_info *info = get_error_info(false);
    if (info && info->signaled && info->uncaught_error_fun) {
        info->uncaught_error_fun(info->error, info->msg);
    }

    if (!plat_rbtk_error_terminate()) {
        return false;
    }

    truncate_msg_len = 0;

    initialized = false;
    return true;
}

void
rbtk_set_uncaught_error_callback(rbtk_uncaught_error_callback_fun callback)
{
    priv_rbtk_error_info *info = get_error_info(true);
    info->uncaught_error_fun = callback;
}

RBTK_ERROR_CODE
rbtk_get_last_error(const char **msg)
{
    if (!initialized) {
        if (*msg) {
            *msg = NULL;
        }
        return RBTK_ERROR_NONE;
    }

    priv_rbtk_error_info *info = get_error_info(false);
    if (!info || !info->signaled) {
        if (msg) {
            *msg = NULL;
        }
        return RBTK_ERROR_NONE;
    }
    else {
        info->signaled = false;
        if (msg) {
            *msg = info->msg;
        }
        return info->error;
    }
}

void
rbtk_signal_error_rt(RBTK_ERROR_CODE error,
    const char *error_name, const char *msg, ...)
{
    assert(initialized);
    assert(error != RBTK_ERROR_NONE);

    priv_rbtk_error_info *info = get_error_info(true);
    if (info->signaled && info->uncaught_error_fun) {
        info->uncaught_error_fun(info->error, info->msg);
    }

    info->signaled = true;
    info->error = error;

     fprintf(stderr, "Error code 0x%x8 [%s] signalled.\n",
        error, error_name);

    if (!msg) {
        info->msg[0] = '\0';
        return; /* nothing to format */
    }

    va_list format;
    va_start(format, msg);
    int print_result = vsnprintf(info->msg,
        RBTK_ERROR_MESSAGE_MAX_LENGTH, msg, format);
    va_end(format);

    /*
     * If the result of printing resulted in truncation, then write
     * over the end of the error string to notify the user that the
     * error message has been truncated.
     */
    if (print_result == -1) {
        size_t truncate_off = RBTK_ERROR_MESSAGE_MAX_LENGTH;
        truncate_off -= truncate_msg_len;
        truncate_off -= 1; /* account for NULL terminator */

        /* insert space if one isn't already there */
        long long msg_end = truncate_off - 1;
        if (msg_end >= 0 && !isspace(info->msg[msg_end])) {
            info->msg[msg_end] = ' ';
        }

        if (truncate_off >= 0) {
            char *dest = info->msg + truncate_off;
            size_t len = RBTK_ERROR_MESSAGE_MAX_LENGTH - truncate_off;
            strncpy(dest, truncate_msg, len - 1);
            dest[len] = '\0'; /* just to be safe */
        }
    }

    fprintf(stderr, "%s", info->msg);

    /* print a newline if one is absent */
    size_t msg_len = strlen(msg);
    if (msg[msg_len] != '\n') {
        fprintf(stderr, "\n");
    }
}

void
rbtk_signal_error_at(RBTK_ERROR_CODE error,
    const char *error_name, const char *msg, ...)
{
    assert(initialized);
    assert(error != RBTK_ERROR_NONE);

    fprintf(stderr, "Error code 0x%x8 [%s] signalled.\n",
        error, error_name);

    if (msg) {
        va_list format;
        va_start(format, msg);
        vfprintf(stderr, msg, format);
        va_end(format);

        /* print a newline if one is absent */
        size_t msg_len = strlen(msg);
        if (msg[msg_len] != '\n') {
            fprintf(stderr, "\n");
        }
    }
}

bool
rbtk_suggest_error(RBTK_ERROR_CODE error, const char *msg, ...)
{
    assert(initialized);
    assert(error != RBTK_ERROR_NONE);

    priv_rbtk_error_info *info = get_error_info(true);
    if (info->signaled) {
        return false; /* another error already signalled */
    }

    va_list format;
    va_start(format, msg);
    rbtk_signal_error(error, msg, format);
    va_end(format);

    return true;
}

void
rbtk_abort_if_error(void)
{
    const char *msg = NULL;
    int code = rbtk_get_last_error(&msg);
    if (code) {
        if (!msg) {
            msg = "(no message)";
        }
        fprintf(stderr, "Error %d: %s\n", code, msg);
        abort();
    }
}
