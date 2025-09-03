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
#include "common.h"
#include "./private/common.h"
#include "./platform/common.h"

#include <stdint.h>

int_least8_t
rbtk_clamp_i8(int_least8_t val, int_least8_t min, int_least8_t max)
{
    const int_least8_t t = val < min ? min : val;
    return t > max ? max : t;
}

uint_least8_t
rbtk_clamp_u8(uint_least8_t val, uint_least8_t min, uint_least8_t max)
{
    const uint_least8_t t = val < min ? min : val;
    return t > max ? max : t;
}

int_least16_t
rbtk_clamp_i16(int_least16_t val, int_least16_t min, int_least16_t max)
{
    const int_least16_t t = val < min ? min : val;
    return t > max ? max : t;
}

uint_least16_t
rbtk_clamp_u16(uint_least16_t val, uint_least16_t min, uint_least16_t max)
{
    const uint_least16_t t = val < min ? min : val;
    return t > max ? max : t;
}

int_least32_t
rbtk_clamp_i32(int_least32_t val, int_least32_t min, int_least32_t max)
{
    const int_least32_t t = val < min ? min : val;
    return t > max ? max : t;
}

uint_least32_t
rbtk_clamp_u32(uint_least32_t val, uint_least32_t min, uint_least32_t max)
{
    const uint_least32_t t = val < min ? min : val;
    return t > max ? max : t;
}

int_least64_t
rbtk_clamp_i64(int_least64_t val, int_least64_t min, int_least64_t max)
{
    const int_least64_t t = val < min ? min : val;
    return t > max ? max : t;
}

uint_least64_t
rbtk_clamp_u64(uint_least64_t val, uint_least64_t min, uint_least64_t max)
{
    const uint_least64_t t = val < min ? min : val;
    return t > max ? max : t;
}

float
rbtk_clamp_f32(float val, float min, float max)
{
    const float t = val < min ? min : val;
    return t > max ? max : t;
}

double
rbtk_clamp_f64(double val, double min, double max)
{
    const double t = val < min ? min : val;
    return t > max ? max : t;
}

long double
rbtk_clamp_f80(long double val, long double min, long double max)
{
    const long double t = val < min ? min : val;
    return t > max ? max : t;
}
