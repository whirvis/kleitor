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
#if defined(_WIN32) || defined(__linux__)

#include "asset.h"

#include <stdbool.h>
#include <string.h>

#include "../error.h"
#include "../stream.h"

RBTK_PLATFORM
typedef struct PLAT_RBTK_ASSET {
	const char *path;
	size_t path_len;
} PLAT_RBTK_ASSET;

static const char *ASSET_DIR_PATH = "./assets/";

static bool initialized;
static size_t asset_dir_path_len;

RBTK_PLATFORM RBTK_NO_DISCARD bool
plat_rbtk_asset_init(void)
{
	if (initialized) {
		return true;
	}

	if (asset_dir_path_len == 0) {
		asset_dir_path_len = strlen(ASSET_DIR_PATH);
	}

	initialized = true;
	return true;
}

RBTK_PLATFORM RBTK_NO_DISCARD bool
plat_rbtk_asset_terminate(void)
{
	if (!initialized) {
		return true;
	}

	initialized = false;
	return true;
}

RBTK_PLATFORM RBTK_NO_DISCARD PLAT_RBTK_ASSET *
plat_rbtk_load_asset(const char *name)
{
	assert(initialized);
	assert(name);

	PLAT_RBTK_ASSET *plat;
	RBTK_MALLOC_OR_RETURN(&plat, NULL, NULL);
	plat->path = name;
	plat->path_len = strlen(name);

	return plat;
}

RBTK_PLATFORM RBTK_NO_DISCARD RBTK_IN_STREAM *
plat_rbtk_open_asset_in_stream(RBTK_ASSET *asset)
{
	assert(initialized);
	assert(asset);

	PLAT_RBTK_ASSET *plat = asset->plat;

	/* add one extra byte for the NULL terminator */
	size_t full_path_len = asset_dir_path_len + plat->path_len + 1;
	char *full_path = malloc(full_path_len);
	if (!full_path) {
		rbtk_signal_error(RBTK_ERROR_OUT_OF_MEMORY,
			"could not allocate memory for asset path string");
		return NULL;
	}
	snprintf(full_path, full_path_len, "%s%s", ASSET_DIR_PATH, plat->path);
	full_path[full_path_len - 1] = '\0'; /* just to be safe */
	
	RBTK_IN_STREAM *in = rbtk_open_file_in_stream(full_path);
	free(full_path);
	return in;
}

#endif /* defined(_WIN32) || defined(__linux__) */
