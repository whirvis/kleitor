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
#ifndef RBTK_PRIVATE_ERROR_H_
#define RBTK_PRIVATE_ERROR_H_

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "../error.h"

#include <stdbool.h>

#include "../common.h"

RBTK_PRIVATE
typedef struct priv_rbtk_error_info {
    bool signaled;
    RBTK_ERROR_CODE error;
    char msg[RBTK_ERROR_MESSAGE_MAX_LENGTH];
    rbtk_uncaught_error_callback_fun uncaught_error_fun;
} priv_rbtk_error_info;

RBTK_PRIVATE RBTK_NO_DISCARD bool
priv_rbtk_error_init(void);

RBTK_PRIVATE RBTK_NO_DISCARD bool
priv_rbtk_error_terminate(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* RBTK_PRIVATE_ERROR_H_ */
