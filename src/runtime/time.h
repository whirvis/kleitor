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
#ifndef RBTK_TIME_H_
#define RBTK_TIME_H_

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/*!
 * @file
 * @brief The public API for the program's time module.
 */

#include <stdbool.h>

#include "common.h"

/*!
 * @defgroup time Precision Timing
 *
 * @brief The program's time module.
 *
 * @see rbtk_time(rbtk_time_unit)
 * @see rbtk_sleep(rbtk_time_unit, long long)
 *
 * @{
 */

/*!
 * @brief Describes a unit of time.
 *
 * @note When storing chronological values, use either `long double` or
 * `long long`.
 */
typedef enum rbtk_time_unit {
    RBTK_NANOS,  /*!< Nanoseconds  (one billionth of a second).  */
    RBTK_MICROS, /*!< Microseconds (one millionth of a second).  */
    RBTK_MILLIS, /*!< Milliseconds (one thousandth of a second). */
    RBTK_SECS,   /*!< Seconds.                                   */
    RBTK_MINS,   /*!< Minutes      (sixty seconds).              */
    RBTK_HOURS,  /*!< Hours        (sixty minutes).              */
    RBTK_DAYS,   /*!< Days         (twenty-four hours).          */
} rbtk_time_unit;

/*!
 * @brief Converts from one unit of time to another.
 *
 * This function is suggested whenever a conversion may be necessary, in
 * order to reduce code redundancies. If the arguments for `from` and `to`
 * are identical this function simply returns `time` as-is.
 *
 * @param[in] from What to convert `time` from.
 * @param[in] to   What to convert `time` to.
 * @param[in] time The chronological value to convert.
 * @return The converted chronological value.
 */
RBTK_NO_DISCARD long double
rbtk_convert_time(rbtk_time_unit from, rbtk_time_unit to,
    long double time);

/*!
 * @brief Returns the current time on this machine.
 *
 * The starting epoch is guaranteed to be January 1st, 1970 at midnight.
 *
 * @note Some processors have limits on the precision of their internal
 * clocks. For example, some processors only have microsecond precision.
 * When the requested precision is greater than what is available, the
 * result will be modified appropriately.<br>
 * For example: if the requested precision is #RBTK_NANOS but the clock
 * only has microsecond precision, the clock time will be multiplied by
 * one thousand before it is returned.
 *
 * @param[in] unit What to return the current time as.
 * @return The current time on this machine.
 */
RBTK_NO_DISCARD long double
rbtk_time(rbtk_time_unit unit);

/*!
 * @brief Pauses the calling thread for the given amount of time.
 *
 * @note Due to the nature of timing, this function **cannot** guarantee the
 * thread will sleep for exactly the specified amount of time. However, it
 * *will* sleep for at least that amount of time.<br>
 * For example: the thread may pause for `511` milliseconds if the argument
 * for `duration` is `500`.
 *
 * This function does not signal #RBTK_ERROR_INTERRUPTED since the relevant
 * information is communicated in its return value.
 *
 * @param[in] unit     How to interpret `duration`.
 * @param[in] duration The amount of time to pause for, a negative value is
 *                     permitted and results in a no-op. However, a value of
 *                     `false` may still be returned.
 * @return `true` if the calling thread slept for the entire duration without
 * being interrupted, `false` otherwise.
 */
bool
rbtk_sleep(rbtk_time_unit unit, long long duration);

/*! @} */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* RBTK_TIME_H_ */
