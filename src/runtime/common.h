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
#ifndef RBTK_COMMON_H_
#define RBTK_COMMON_H_

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/*!
 * @file
 * @brief The commons for the entire program.
 *
 * @attention This file should be included by **all other headers.**
 */

#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "error.h"

/*!
 * @defgroup common Commons
 *
 * @brief The program's common utilities.
 *
 * This module contains functions, definitions, etc. used by the entire
 * program. This exists so the public and private APIs can access common
 * parts used by both. The header for this module should be included by
 * all other public API headers.
 *
 * @{
 */

#if defined(__GNUC__) && (__GNUC__ >= 4)
#define RBTK_NO_DISCARD __attribute__ ((warn_unused_result))
#elif defined(_MSC_VER) && (_MSC_VER >= 1700)
#define RBTK_NO_DISCARD _Check_return_
#else
#define RBTK_NO_DISCARD
#endif

/*!
 * @def RBTK_NO_DISCARD
 * @brief Indicates a function's return value should be used.
 *
 * An example of this would be a function that opens a file or will likely
 * encounter an error. This should be added to functions in which ignoring
 * the return value will likely result in some other issue.
 *
 * @see https://stackoverflow.com/a/22759336/4531148
 */

#if defined(__GNUC__) && (__GNUC__ >= 3)
#define RBTK_UNUSED __attribute__((__unused__))
#else
#define RBTK_UNUSED __pragma(warning(suppress:4100))
#endif

/*!
 * @def RBTK_UNUSED
 * @brief Indicates a function or variable may intentionally be unused.
 * 
 * An example of this would be a function which acts as a no-op for some
 * operation. If it takes in any parameters, it is very likely not to use
 * them. However, they are still required in order for the declaration to
 * be valid.
 */

/*!
 * @brief Indicates something should be hidden from users.
 *
 * @attention Wherever possible, `static` linkage should be used instead
 * to achieve actual privacy.<br>
 * Furthermore, anything with this attribute should *not* be in headers
 * which are included by the user.
 *
 * @note While not required, it is considered good practice to prepend
 * `priv_` (with desired casing) to anything with this attribute.
 *
 * @see RBTK_PLATFORM
 */
#define RBTK_PRIVATE

/*!
 * @brief Indicates platform specific behavior.
 *
 * Anything with this attribute is (usually) wrapped by a related function or
 * type which will perform operations or store data that should be present on
 * every machine regardless of platform.
 *
 * @note While not required, it is considered good practice to prepend
 * `plat_` (with desired casing) to anything with this attribute.
 *
 * @see RBTK_PRIVATE
 */
#define RBTK_PLATFORM

/*!
 * @brief Indicates a forward declaration.
 *
 * These occur wherever it is not possible to include a header file, but
 * a type must still be referenced. Circular includes (dependencies) are
 * a common cause for this.
 *
 * @note Forward declarations should not have a body.
 */
#define RBTK_FORWARD_DECLARATION

/*!
 * @brief Indicates a function requires an implementation.
 *
 * @warning Functions declarations with this attribute *must* be assigned
 * a valid function pointer which matches their prototype. Failure to do
 * so will result in undefined behavior on invocation. Whether #RBTK_NO_OP,
 * #RBTK_DEFAULT_IMPL, or #RBTK_UNIMPLEMENTED are allowed or not depends
 * on the caller's requirements.
 *
 * @see RBTK_FUNC_IS_IMPLEMENTED(_func_ptr)
 */
#define RBTK_ABSTRACT_FUNC

/*!
 * @brief Indicates a function has a default implementation.
 *
 * @see RBTK_DEFAULT_IMPL
 */
#define RBTK_DEFAULT_FUNC

/*!
 * @brief Pointer which signifies a function is a no-op.
 *
 * @note Function pointers initialized to this value does not need to be
 * assigned to a function. However, if they are, said function must match
 * the prototype of the expected function.
 *
 * @see RBTK_DEFAULT_IMPL
 * @see RBTK_UNIMPLEMENTED
 */
#define RBTK_NO_OP         ((void (*)(void)) 0)

/*!
 * @brief Pointer which signifies a function is not implemented.
 *
 * @note Function pointers initialized to this value will eventually be
 * re-assigned to `NULL` and will never be called.
 *
 * This differs from #RBTK_NO_OP in that #RBTK_ERROR_UNSUPPORTED will be
 * signaled whenever the function would have been invoked. This should be
 * used when a no-op would result in unexpected behavior (e.g., trying to
 * skip over bytes in a stream that does not support it).
 *
 * @see RBTK_DEFAULT_IMPL
 */
#define RBTK_UNIMPLEMENTED ((void (*)(void)) (SIZE_MAX - 0))

/*!
 * @brief Pointer which signifies a default implementation.
 *
 * @note Function pointers initialized to this value will eventually be
 * re-assigned to a valid function pointer.
 *
 * @see RBTK_DEFAULT_FUNC
 */
#define RBTK_DEFAULT_IMPL  ((void (*)(void)) (SIZE_MAX - 1))

/*!
 * @brief Determines if a pointer likely points to a real function.
 *
 * @attention Function pointers are assumed to be valid so long as they
 * are not equal to #RBTK_UNIMPLEMENTED.
 *
 * @param[in] _func_ptr the function pointer.
 */
#define RBTK_FUNC_IS_IMPLEMENTED(_func_ptr) \
   ((_func_ptr) != RBTK_ABSTRACT)

/*!
 * @brief Zero initializes a block of memory.
 *
 * @attention For this macro to function correctly, it must be possible for
 * the caller to invoke `sizeof(*(_block))` in its current scope.
 *
 * @param[in,out] _block The memory to zero initialize.
 *
 * @debugging This macro asserts that `_data` is not `NULL`.
 */
#define RBTK_ZERO_MEMORY(_block)              \
    assert((_block));                         \
    memset((_block), 0x00, sizeof(*(_block))) \

/*!
 * @brief Allocates memory for a type or returns a fallback value.
 *
 * If allocation fails, the given fallback value will be returned. This macro
 * uses `malloc(size_t)` to allocate the requested memory.
 *
 * @note This macro should only be used from within a function.
 *
 * @param[in] _dest     A pointer to the pointer which shall store the address
                        of the allocated memory. This pointer shall be double
                        de-referenced to determine how much memory to allocate.
                        If allocation fails, it will point to `NULL`.
 * @param[in] _fallback The value to return on failure.
 * @param[in] _msg      The error message to use on failure, may be `NULL` for
 *                      a default error message.
 * @param[in] ...       Format arguments for `_msg`, if any.
 *
 * @debugging This macro asserts that `_dest` is not `NULL`.
 * @errors
 * @signal{#RBTK_ERROR_OUT_OF_MEMORY, If the memory could not be allocated.}
 * @enderrors
 *
 * @pointer_lifetime The memory allocated by this macro is the caller's
 * responsibility.
 *
 * @see rbtk_get_last_error(const char **)
 * @see rbtk_signal_error(RBTK_ERROR_CODE, const char *, ...)
 */
#define RBTK_MALLOC_OR_RETURN(_dest, _fallback, _msg, ...)     \
    do {                                                       \
        assert((_dest));                                       \
                                                               \
        size_t data_size = sizeof(**(_dest));                  \
        *(_dest) = malloc(data_size);                          \
                                                               \
        if (!*(_dest)) {                                       \
            if ((_msg)) {                                      \
                rbtk_signal_error(RBTK_ERROR_OUT_OF_MEMORY,    \
                    (_msg), ##__VA_ARGS__);                    \
            } else {                                           \
                rbtk_signal_error(RBTK_ERROR_OUT_OF_MEMORY,    \
                    "failed to allocate %d bytes", data_size); \
            }                                                  \
            return (_fallback);                                \
        }                                                      \
    } while (0)

/*!
 * @brief Inserts an element at the end of a doubly-linked list.
 *
 * For this macro to expand correctly, `_head`, `_tail`, and `_elem` must
 * all be of the same type. Furthermore, the type must have fields named
 * `prev` and `next`. These fields must be the same type as the arguments
 * passed to this macro.
 *
 * @param[in] _head the head of the doubly-linked list.
 * @param[in] _tail the tail of the doubly-linked list.
 * @param[in] _elem the element to insert.
 *
 * @see RBTK_DLL_REMOVE(_head, _tail, _elem)
 */
#define RBTK_DLL_PUSH(_head, _tail, _elem) \
    do {                                   \
        if (!(_head)) {                    \
            (_elem)->prev = NULL;          \
            (_elem)->next = NULL;          \
            (_head)       = (_elem);       \
            (_tail)       = (_elem);       \
        } else {                           \
            (_elem)->prev = (_tail);       \
            (_tail)->next = (_elem);       \
            (_tail)       = (_elem);       \
        }                                  \
    } while (0)

/*!
 * @brief Removes an element from a doubly-linked list.
 *
 * For this macro to expand correctly, `_head`, `_tail`, and `_elem` must
 * all be of the same type. Furthermore, the type must have fields named
 * `prev` and `next`. These fields must be the same type as the arguments
 * passed to this macro.
 *
 * @param[in] _head the head of the doubly-linked list.
 * @param[in] _tail the tail of the doubly-linked list.
 * @param[in] _elem the element to remove.
 *
 * @see RBTK_DLL_PUSH(_head, _tail, _elem)
 */
#define RBTK_DLL_REMOVE(_head, _tail, _elem)     \
    do {                                         \
        if ((_elem) == (_head)) {                \
            (_head) = (_elem)->next;             \
        }                                        \
        if ((_elem) == (_tail)) {                \
            (_tail) = (_elem)->prev;             \
        }                                        \
        if ((_elem)->prev) {                     \
            (_elem)->prev->next = (_elem)->next; \
        }                                        \
        if ((_elem)->next) {                     \
            (_elem)->next->prev = (_elem)->prev; \
        }                                        \
    } while(0)

/*!
 * @brief Clamps a signed 8-bit integer.
 *
 * If the given value is lower than the given minimum, the minimum value
 * is returned. If the given value is greater than the given maximum, then
 * the maximum value is returned. Otherwise, the given value is returned.
 *
 * This code is taken from [Stack Overflow.](https://stackoverflow.com\
 * /a/16659263/4531148) It is used as the original author claims that both
 * GCC and clang generate optimized assembly code for this purpose. It is
 * left unmodified (except for the type) to ensure the mentioned assembly
 * is generated.
 *
 * @param[in] val The value to clamp.
 * @param[in] min The minimum value to return.
 * @param[in] max The maximum value to return.
 *
 * @see rbtk_clamp_u8(uint_least8_t, uint_least8_t, uint_least8_t)
 */
int_least8_t
rbtk_clamp_i8(int_least8_t val, int_least8_t min, int_least8_t max);

/*!
 * @brief Clamps an unsigned 8-bit integer.
 *
 * If the given value is lower than the given minimum, the minimum value
 * is returned. If the given value is greater than the given maximum, then
 * the maximum value is returned. Otherwise, the given value is returned.
 *
 * This code is taken from [Stack Overflow.](https://stackoverflow.com\
 * /a/16659263/4531148) It is used as the original author claims that both
 * GCC and clang generate optimized assembly code for this purpose. It is
 * left unmodified (except for the type) to ensure the mentioned assembly
 * is generated.
 *
 * @param[in] val The value to clamp.
 * @param[in] min The minimum value to return.
 * @param[in] max The maximum value to return.
 *
 * @see rbtk_clamp_i8(int_least8_t, int_least8_t, int_least8_t)
 */
uint_least8_t
rbtk_clamp_u8(uint_least8_t val, uint_least8_t min, uint_least8_t max);

/*!
 * @brief Clamps a signed 16-bit integer.
 *
 * If the given value is lower than the given minimum, the minimum value
 * is returned. If the given value is greater than the given maximum, then
 * the maximum value is returned. Otherwise, the given value is returned.
 *
 * This code is taken from [Stack Overflow.](https://stackoverflow.com\
 * /a/16659263/4531148) It is used as the original author claims that both
 * GCC and clang generate optimized assembly code for this purpose. It is
 * left unmodified (except for the type) to ensure the mentioned assembly
 * is generated.
 *
 * @param[in] val The value to clamp.
 * @param[in] min The minimum value to return.
 * @param[in] max The maximum value to return.
 *
 * @see rbtk_clamp_u16(uint_least16_t, uint_least16_t, uint_least16_t)
 */
int_least16_t
rbtk_clamp_i16(int_least16_t val, int_least16_t min, int_least16_t max);

/*!
 * @brief Clamps an unsigned 16-bit integer.
 *
 * If the given value is lower than the given minimum, the minimum value
 * is returned. If the given value is greater than the given maximum, then
 * the maximum value is returned. Otherwise, the given value is returned.
 *
 * This code is taken from [Stack Overflow.](https://stackoverflow.com\
 * /a/16659263/4531148) It is used as the original author claims that both
 * GCC and clang generate optimized assembly code for this purpose. It is
 * left unmodified (except for the type) to ensure the mentioned assembly
 * is generated.
 *
 * @param[in] val The value to clamp.
 * @param[in] min The minimum value to return.
 * @param[in] max The maximum value to return.
 *
 * @see rbtk_clamp_i16(int_least16_t, int_least16_t, int_least16_t)
 */
uint_least16_t
rbtk_clamp_u16(uint_least16_t val, uint_least16_t min, uint_least16_t max);

/*!
 * @brief Clamps a signed 32-bit integer.
 *
 * If the given value is lower than the given minimum, the minimum value
 * is returned. If the given value is greater than the given maximum, then
 * the maximum value is returned. Otherwise, the given value is returned.
 *
 * This code is taken from [Stack Overflow.](https://stackoverflow.com\
 * /a/16659263/4531148) It is used as the original author claims that both
 * GCC and clang generate optimized assembly code for this purpose. It is
 * left unmodified (except for the type) to ensure the mentioned assembly
 * is generated.
 *
 * @param[in] val The value to clamp.
 * @param[in] min The minimum value to return.
 * @param[in] max The maximum value to return.
 *
 * @see rbtk_clamp_u32(uint_least32_t, uint_least32_t, uint_least32_t)
 */
int_least32_t
rbtk_clamp_i32(int_least32_t val, int_least32_t min, int_least32_t max);

/*!
 * @brief Clamps an unsigned 32-bit integer.
 *
 * If the given value is lower than the given minimum, the minimum value
 * is returned. If the given value is greater than the given maximum, then
 * the maximum value is returned. Otherwise, the given value is returned.
 *
 * This code is taken from [Stack Overflow.](https://stackoverflow.com\
 * /a/16659263/4531148) It is used as the original author claims that both
 * GCC and clang generate optimized assembly code for this purpose. It is
 * left unmodified (except for the type) to ensure the mentioned assembly
 * is generated.
 *
 * @param[in] val The value to clamp.
 * @param[in] min The minimum value to return.
 * @param[in] max The maximum value to return.
 *
 * @see rbtk_clamp_i32(int_least32_t, int_least32_t, int_least32_t)
 */
uint_least32_t
rbtk_clamp_u32(uint_least32_t val, uint_least32_t min, uint_least32_t max);

/*!
 * @brief Clamps a signed 64-bit integer.
 *
 * If the given value is lower than the given minimum, the minimum value
 * is returned. If the given value is greater than the given maximum, then
 * the maximum value is returned. Otherwise, the given value is returned.
 *
 * This code is taken from [Stack Overflow.](https://stackoverflow.com\
 * /a/16659263/4531148) It is used as the original author claims that both
 * GCC and clang generate optimized assembly code for this purpose. It is
 * left unmodified (except for the type) to ensure the mentioned assembly
 * is generated.
 *
 * @param[in] val The value to clamp.
 * @param[in] min The minimum value to return.
 * @param[in] max The maximum value to return.
 *
 * @see rbtk_clamp_u64(uint_least64_t, uint_least64_t, uint_least64_t)
 */
int_least64_t
rbtk_clamp_i64(int_least64_t val, int_least64_t min, int_least64_t max);

/*!
 * @brief Clamps an unsigned 64-bit integer.
 *
 * If the given value is lower than the given minimum, the minimum value
 * is returned. If the given value is greater than the given maximum, then
 * the maximum value is returned. Otherwise, the given value is returned.
 *
 * This code is taken from [Stack Overflow.](https://stackoverflow.com\
 * /a/16659263/4531148) It is used as the original author claims that both
 * GCC and clang generate optimized assembly code for this purpose. It is
 * left unmodified (except for the type) to ensure the mentioned assembly
 * is generated.
 *
 * @param[in] val The value to clamp.
 * @param[in] min The minimum value to return.
 * @param[in] max The maximum value to return.
 *
 * @see rbtk_clamp_i64(int_least64_t, int_least64_t, int_least64_t)
 */
uint_least64_t
rbtk_clamp_u64(uint_least64_t val, uint_least64_t min, uint_least64_t max);

/*!
 * @brief Clamps a 32-bit float point value.
 *
 * If the given value is lower than the given minimum, the minimum value
 * is returned. If the given value is greater than the given maximum, then
 * the maximum value is returned. Otherwise, the given value is returned.
 *
 * This code is taken from [Stack Overflow.](https://stackoverflow.com\
 * /a/16659263/4531148) It is used as the original author claims that both
 * GCC and clang generate optimized assembly code for this purpose. It is
 * left unmodified (except for the type) to ensure the mentioned assembly
 * is generated.
 *
 * @param[in] val The value to clamp.
 * @param[in] min The minimum value to return.
 * @param[in] max The maximum value to return.
 */
float
rbtk_clamp_f32(float val, float min, float max);

/*!
 * @brief Clamps a 64-bit float point value.
 *
 * If the given value is lower than the given minimum, the minimum value
 * is returned. If the given value is greater than the given maximum, then
 * the maximum value is returned. Otherwise, the given value is returned.
 *
 * This code is taken from [Stack Overflow.](https://stackoverflow.com\
 * /a/16659263/4531148) It is used as the original author claims that both
 * GCC and clang generate optimized assembly code for this purpose. It is
 * left unmodified (except for the type) to ensure the mentioned assembly
 * is generated.
 *
 * @param[in] val The value to clamp.
 * @param[in] min The minimum value to return.
 * @param[in] max The maximum value to return.
 */
double
rbtk_clamp_f64(double val, double min, double max);

/*!
 * @brief Clamps an 80-bit float point value.
 *
 * If the given value is lower than the given minimum, the minimum value
 * is returned. If the given value is greater than the given maximum, then
 * the maximum value is returned. Otherwise, the given value is returned.
 *
 * This code is taken from [Stack Overflow.](https://stackoverflow.com\
 * /a/16659263/4531148) It is used as the original author claims that both
 * GCC and clang generate optimized assembly code for this purpose. It is
 * left unmodified (except for the type) to ensure the mentioned assembly
 * is generated.
 *
 * @param[in] val The value to clamp.
 * @param[in] min The minimum value to return.
 * @param[in] max The maximum value to return.
 */
long double
rbtk_clamp_f80(long double val, long double min, long double max);

/*! @} */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* RBTK_COMMON_H_ */
