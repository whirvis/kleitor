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
#ifndef RBTK_RUNTIME_H_
#define RBTK_RUNTIME_H_

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/*!
 * @file
 * @brief The basis for the program's runtime.
 */

#include <stdbool.h>

#include "asset.h"
#include "common.h"
#include "error.h"
#include "thread.h"

/*!
 * @defgroup runtime Program Runtime.
 *
 * @attention This runtime expects #rbtk_runtime_main(int, const char *[])
 * to be implemented by another file. If not defined, linking will fail.
 *
 * @brief The program's runtime.
 * @pre   Initialize the runtime.
 * @post  Terminate the runtime.
 *
 * @see rbtk_init_runtime(void)
 * @see rbtk_terminate_runtime(void)
 *
 * @{
 */

/*!
 * @brief Initializes the program's runtime.
 *
 * This consists of initilizing all of the program's modules so they are
 * immediately ready to use. If initialization fails, the program aborts
 * immediately.
 */
void
rbtk_init_runtime(void);

/*!
 * @brief Terminates the program's runtime.
 *
 * This consists of terminates all of the program's modules so they are no
 * longer available. If termination fails, the program aborts immediately.
 */
void
rbtk_terminate_runtime(void);

/*!
 * @brief The entrypoint for programs using this runtime.
 *
 * @param[in] argc The number of arguments.
 * @param[in] argv The arguments for each value.
 * @return The result of execution.
 */
RBTK_NO_DISCARD int
rbtk_runtime_main(int argc, const char *argv[]);

/*! @} */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* RBTK_RUNTIME_H_ */
