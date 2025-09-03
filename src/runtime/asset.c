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
#include "asset.h"
#include "./private/asset.h"
#include "./platform/asset.h"

#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>

#include "common.h"
#include "error.h"
#include "stream.h"

#define REQUIRE_INITIALIZED_OR_RETURN(_value)       \
    if (!initialized) {                             \
        rbtk_signal_error(RBTK_ERROR_ILLEGAL_STATE, \
            "asset module not initialized");        \
        return (_value);                            \
    }

struct loaded_asset {
    RBTK_ASSET *asset;
    struct loaded_asset *prev;
    struct loaded_asset *next;
};

static struct loaded_asset *assets_head;
static struct loaded_asset *assets_tail;
static bool initialized;

static RBTK_ASSET *
load_asset(const char *name)
{
    assert(name);

    /* the path cannot start or end with a forward slash */
    assert(name[0] != '/');
    size_t name_len = strlen(name);
    assert(name[name_len - 1] != '/');

    /* copy the name to avoid pointer lifetime issues */
    char *name_copy = malloc(name_len + 1);
    if (!name_copy) {
        rbtk_signal_error(RBTK_ERROR_OUT_OF_MEMORY,
            "failed to allocate memory for asset name");
        return NULL;
    }
    else {
        strncpy(name_copy, name, name_len);
        name_copy[name_len] = '\0'; /* just to be safe */
    }

    PLAT_RBTK_ASSET *plat = plat_rbtk_load_asset(name);
    if (!plat) {
        free(name_copy);
        return NULL;
    }

    RBTK_ASSET *asset = malloc(sizeof(*asset));
    if (!asset) {
        free(name_copy);
        free(plat);
        rbtk_signal_error(RBTK_ERROR_OUT_OF_MEMORY,
            "failed to allocate memory for asset");
        return NULL;
    }

    asset->plat = plat;
    asset->name = name_copy;
    asset->name_len = name_len;

    struct loaded_asset *loaded = malloc(sizeof(*loaded));
    if (!loaded) {
        rbtk_signal_error(RBTK_ERROR_OUT_OF_MEMORY,
            "failed to allocate memory for loaded asset");
        free(loaded);
        return NULL;
    }

    loaded->asset = asset;
    loaded->prev = NULL;
    loaded->next = NULL;

    RBTK_DLL_PUSH(assets_head, assets_tail, loaded);
    return asset;
}

RBTK_PRIVATE RBTK_NO_DISCARD bool
priv_rbtk_asset_init(void)
{
    if (initialized) {
        return true;
    }

    if (!plat_rbtk_asset_init()) {
        return false;
    }

    initialized = true;
    return true;
}

RBTK_PRIVATE RBTK_NO_DISCARD bool
priv_rbtk_asset_terminate(void)
{
    if (!initialized) {
        return true;
    }

    if (!plat_rbtk_asset_terminate()) {
        return false;
    }

    initialized = false;
    return true;
}

RBTK_NO_DISCARD RBTK_ASSET *
rbtk_get_asset(const char *name)
{
    assert(name);

    REQUIRE_INITIALIZED_OR_RETURN(NULL);

    RBTK_ASSET *located = NULL;

    struct loaded_asset *cur = assets_head;
    while (cur) {
        RBTK_ASSET *asset = cur->asset;
        if (!strcmp(asset->name, name)) {
            located = asset;
            break;
        }
        cur = cur->next;
    }

    if (!located) {
        return load_asset(name);
    }

    return located;
}

RBTK_NO_DISCARD RBTK_ASSET *
rbtk_require_asset(const char *name)
{
    RBTK_ASSET *asset = rbtk_get_asset(name);
    if (!asset) {
        rbtk_signal_error(RBTK_ERROR_IO,
            "could not locate asset %s", name);
        rbtk_abort_if_error();
    }
    return asset;
}

RBTK_NO_DISCARD const char *
rbtk_get_asset_name(RBTK_ASSET *asset)
{
    assert(asset);
    return asset->name;
}

RBTK_NO_DISCARD RBTK_IN_STREAM *
rbtk_open_asset_in_stream(RBTK_ASSET *asset)
{
    assert(asset);
    return plat_rbtk_open_asset_in_stream(asset);
}
