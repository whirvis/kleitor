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
#ifndef RBTK_LIBRARIES_CGLM_H_
#define RBTK_LIBRARIES_CGLM_H_

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/*!
 * @file
 * @brief Forwarder for the CGLM library.
 *
 * This forwarder exists as the CGLM library uses some functions in its
 * I/O module which Visual Studio doesn't like. As such, the IDE refuses
 * to compile without secure warnings being turned off (which we don't
 * want). Since we don't need the I/O module from CGLM, this forwarder
 * simply excludes it.
 */

/*
 * If its already defined, just include the header. Otherwise, add the
 * necessary define, include the header, and then undefine it. This is
 * to ensure that including this header forwarder doesn't have unwanted
 * side effects.
 */
#ifdef cglm_io_h
#include <cglm/cglm.h>
#else
#define cglm_io_h
#include <cglm/cglm.h>
#undef cglm_io_h
#endif

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* RBTK_LIBRARIES_CGLM_H_ */
