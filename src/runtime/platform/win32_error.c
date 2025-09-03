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
#if defined(_WIN32)

#include "error.h"

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include <windows.h>

static BOOL initialized;
static DWORD error_tls_index;

RBTK_PLATFORM RBTK_NO_DISCARD bool
plat_rbtk_error_init(void)
{
	if (initialized) {
		return true;
	}

	error_tls_index = TlsAlloc();
	if (error_tls_index == TLS_OUT_OF_INDEXES) {
		fprintf(stderr, "Failed to allocate TLS for error info.");
		return false;
	}

	initialized = 1;
	return true;
}

RBTK_PLATFORM RBTK_NO_DISCARD bool
plat_rbtk_error_terminate(void)
{
	if (!initialized) {
		return true;
	}

	if (!TlsFree(error_tls_index)) {
        rbtk_signal_error(RBTK_ERROR_PLATFORM,
            "failure to free TLS for error data.");
		return false;
	}

	error_tls_index = 0;
	initialized = 0;

	return true;
}

RBTK_PLATFORM RBTK_NO_DISCARD priv_rbtk_error_info *
plat_rbtk_get_error_info(bool allocate_if_missing, bool *allocated,
	RBTK_ERROR_CODE *error)
{
	assert(allocated);
	assert(error);

	*allocated = false;
	*error = RBTK_ERROR_NONE;

	priv_rbtk_error_info *info = TlsGetValue(error_tls_index);
	if (!info && allocate_if_missing) {
		DWORD win32_error = GetLastError();
		if(win32_error) {
			SetLastError(win32_error);
			*error = RBTK_ERROR_PLATFORM;
			return NULL;
		}
		
		info = malloc(sizeof(*info));
		if (!info) {
			*error = RBTK_ERROR_OUT_OF_MEMORY;
			return NULL;
		}
		
		TlsSetValue(error_tls_index, info);
		*allocated = true;
	}
	return info;
}

#endif /* defined(_WIN32) */
