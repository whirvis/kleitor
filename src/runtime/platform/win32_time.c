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

#include "time.h"

#include <windows.h>

#include "thread.h"

RBTK_PLATFORM RBTK_NO_DISCARD long double
plat_rbtk_time(rbtk_time_unit unit)
{
    FILETIME file_time;
    GetSystemTimeAsFileTime(&file_time);

    ULARGE_INTEGER system_time = { 0 };
    system_time.LowPart = file_time.dwLowDateTime;
    system_time.HighPart = file_time.dwHighDateTime;

    /*
     * Before converting the time from nanoseconds to the desired unit,
     * we must first do two things.
     *
     * First, we must subtract what looks like a magic number from the
     * system time. This "magic" number actually changes the system epoch
     * time to January 1st, 1970 12:00:00AM. In Windows, the epoch time
     * is January 1st, 1601 12:00:00AM which is not what we want.
     *
     * Secondly, we must multiply the value by 100. This is because the
     * operating system returns the time in hectonanoseconds (that being,
     * 100 nanosecond intervals). This is fixed by converting the value
     * to nanoseconds by simply multiplying by one hundred.
     *
     * The two steps above must be done in the given order.
     */
    system_time.QuadPart -= 116444736000000000LL;
    system_time.QuadPart *= 100;

    return rbtk_convert_time(RBTK_NANOS, unit,
        (long double) system_time.QuadPart);
}

RBTK_PLATFORM RBTK_NO_DISCARD bool
plat_rbtk_sleep(rbtk_time_unit unit, long long duration)
{
    LARGE_INTEGER requested = { 0 };
    requested.QuadPart = (LONGLONG) rbtk_convert_time(unit, RBTK_NANOS,
        (long double) duration);

    LARGE_INTEGER frequency;
    QueryPerformanceFrequency(&frequency);
    LARGE_INTEGER start_time, current_time;
    QueryPerformanceCounter(&start_time);

    LARGE_INTEGER elapsed = { 0 };
    while (elapsed.QuadPart < requested.QuadPart) {
        if (rbtk_thread_interrupted(NULL, true)) {
            return false;
        }
        QueryPerformanceCounter(&current_time);
        elapsed.QuadPart = current_time.QuadPart - start_time.QuadPart;
        elapsed.QuadPart *= 1000000000;
        elapsed.QuadPart /= frequency.QuadPart;
    }

    return true;
}

#endif /* defined(_WIN32) */
