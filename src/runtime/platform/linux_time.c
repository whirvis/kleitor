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
#if defined(__linux__)

#include "time.h"

#include <time.h>

RBTK_PLATFORM RBTK_NO_DISCARD long double
plat_rbtk_time(rbtk_time_unit unit)
{
    struct timespec spec;
    clock_gettime(CLOCK_REALTIME, &spec);
    long double secs = spec.tv_sec;
    secs += (spec.tv_nsec / 1000000000.0l);

    return rbtk_convert_time(RBTK_SECS, unit, secs);
}

RBTK_PLATFORM RBTK_NO_DISCARD bool
plat_rbtk_sleep(rbtk_time_unit unit, long long duration)
{
    int whole_secs = (int) rbtk_convert_time(unit, RBTK_SECS, duration);
    long double nanos = rbtk_convert_time(unit, RBTK_NANOS, duration);
    nanos -= (whole_secs * 1000000000.0l);

    struct timespec spec;
    spec.tv_sec = whole_secs;
    spec.tv_nsec = nanos;

    struct timespec remain = { 0 };
    clock_nanosleep(CLOCK_REALTIME, 0, &spec, &remain);
    return remain.tv_sec > 0 || remain.tv_nsec > 0;
}

#endif /* defined(__linux__) */
