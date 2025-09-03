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
#ifndef RBTK_ASSET_H_
#define RBTK_ASSET_H_

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/*!
 * @file
 * @brief The public API for the program's asset module.
 */

#include "common.h"
#include "stream.h"

/*!
 * @defgroup asset Assets
 *
 * @brief The program's asset module.
 * @pre   Initialize the asset module.
 * @post  Terminate the asset module.
 *
 * This module exists to provide a standard way of accessing assets across
 * different platforms. While most operating systems simply use `fopen()`,
 * some do not. Furthermore, the rules for a path can differentiate between
 * operating systems which do implement it. As such, this module provides
 * types for accessing resources in a platform indepent manner.
 *
 * @see rbtk_get_asset(const char *)
 *
 * @{
 */

/*!
 * @brief Represents an asset in the program.
 *
 * At the moment, assets are read-only. However, a mechanism for writing to
 * assets may be added in the future.
 *
 * @see rbtk_get_asset(const char *)
 */
typedef struct RBTK_ASSET RBTK_ASSET;

/*!
 * @brief Gets an asset.
 * @pre Initialize the asset module.
 *
 * @param[in] name The asset name.
 * @return The retrieved asset, `NULL` if it does not exist.
 *
 * @pointer_lifetime The returned pointer is valid until the asset module is
 * terminated. It is an unchecked runtime error to free it.
 *
 * @debugging This function asserts that `name` is not `NULL`.
 * @errors
 * @signal{#RBTK_ERROR_ILLEGAL_STATE, If the asset module is not initialized.}
 * @signal{#RBTK_ERROR_IO,            If an I/O error occurs.}
 * @enderrors
 *
 * @see rbtk_open_asset_in_stream(RBTK_ASSET *)
 */
RBTK_NO_DISCARD RBTK_ASSET *
rbtk_get_asset(const char *name);

/*!
 * @brief Gets an asset and aborts if it does not exist.
 * @pre Initialize the asset module.
 *
 * @param[in] name The asset name.
 * @return The retrieved asset.
 *
 * @pointer_lifetime The returned pointer is valid until the asset module is
 * terminated. It is an unchecked runtime error to free it.
 *
 * @debugging This function asserts that `name` is not `NULL`.
 * @errors
 * @signal{#RBTK_ERROR_ILLEGAL_STATE, If the asset module is not initialized.}
 * @signal{#RBTK_ERROR_IO,            If an I/O error occurs.}
 * @enderrors
 *
 * @see rbtk_open_asset_in_stream(RBTK_ASSET *)
 */
RBTK_NO_DISCARD RBTK_ASSET *
rbtk_require_asset(const char *name);

/*!
 * @brief Returns the name of an asset.
 *
 * @param[in] asset The asset to query.
 * @return The name of the asset.
 *
 * @pointer_lifetime The returned pointer is valid until the asset module is
 * terminated. It is an unchecked runtime error to free it.
 */
RBTK_NO_DISCARD const char *
rbtk_get_asset_name(RBTK_ASSET *asset);

/*!
 * @brief Opens an input stream for an asset.
 *
 * @param[in] asset The asset to read from.
 * @return The opened stream, `NULL` on error.
 *
 * @pointer_lifetime The returned pointer follow the lifetime rules of an
 * input stream. It is the responsiblity of the caller to close the stream
 * once they are done with it.
 *
 * @debugging This function asserts that `asset` is not `NULL`.
 * @errors
 * @signal{#RBTK_ERROR_IO, If an I/O error occurs.}
 * @enderrors
 */
RBTK_NO_DISCARD RBTK_IN_STREAM *
rbtk_open_asset_in_stream(RBTK_ASSET *asset);

/*! @} */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* RBTK_ASSET_H_ */
