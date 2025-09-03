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
#include "time.h"
#include "./private/time.h"
#include "./platform/time.h"

#include <assert.h>
#include <math.h>
#include <stdbool.h>

#include "thread.h"

RBTK_NO_DISCARD long double
rbtk_convert_time(rbtk_time_unit from, rbtk_time_unit to,
    long double time)
{
    if (from == to) {
        return time; /* no need to convert */
    }
    else if (time == INFINITY || time == -INFINITY) {
        return time; /* cannot convert infinity */
    }

    long double nanos = 0.0l;
    switch (from) {
    case RBTK_NANOS:
        nanos = time;
        break;
    case RBTK_MICROS:
        nanos = time * 1000.0l;
        break;
    case RBTK_MILLIS:
        nanos = time * 1000000.0l;
        break;
    case RBTK_SECS:
        nanos = time * 1000000000.0l;
        break;
    case RBTK_MINS:
        nanos = time * 60000000000.0l;
        break;
    case RBTK_HOURS:
        nanos = time * 3600000000000.0l;
        break;
    case RBTK_DAYS:
        nanos = time * 86400000000000.0l;
        break;
    }

    switch (to) {
    case RBTK_NANOS:
        return nanos;
    case RBTK_MICROS:
        return nanos / 1000.0l;
    case RBTK_MILLIS:
        return nanos / 1000000.0l;
    case RBTK_SECS:
        return nanos / 1000000000.0l;
    case RBTK_MINS:
        return nanos / 60000000000.0l;
    case RBTK_HOURS:
        return nanos / 3600000000000.0l;
    case RBTK_DAYS:
        return nanos / 86400000000000.0l;
    }

    assert(0);    /* we forgot an enum!  */
    return -1.0l; /* pacify the compiler */
}

RBTK_NO_DISCARD long double
rbtk_time(rbtk_time_unit unit)
{
    return plat_rbtk_time(unit);
}

bool
rbtk_sleep(rbtk_time_unit unit, long long duration)
{
    if (duration <= 0) {
        return !rbtk_thread_interrupted(NULL, true);
    }
    return plat_rbtk_sleep(unit, duration);
}
