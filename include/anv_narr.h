/*
 * The MIT License
 *
 * Copyright 2023 Andrea Vouk.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

/*------------------------------------------------------------------------------
   anv_narr (https://github.com/anvouk/anv)
--------------------------------------------------------------------------------



------------------------------------------------------------------------------*/

#ifndef ANV_NARR_H
#define ANV_NARR_H

#include "anv_arr.h"

#include <stdint.h> /* for num types */

#ifdef __cplusplus
extern "C" {
#endif

typedef anv_arr_t anv_narr_t;

typedef union anv_narr_numerical_t {
    char c;
    int i;
    unsigned u;
    long l;
    long long ll;
    int8_t i8;
    uint8_t u8;
    int16_t i16;
    uint16_t u16;
    int32_t i32;
    uint32_t u32;
    int64_t i64;
    uint64_t u64;
    size_t sz;
    float f;
    double d;
} anv_narr_numerical_t;

#define anv_narr_new(initial_capacity)                                         \
    anv_arr_new(initial_capacity, sizeof(anv_narr_numerical_t));

#define anv_narr_destroy(arr) anv_arr_destroy(arr)

#define anv_narr_get(arr, idx) anv_arr_get(arr, anv_narr_numerical_t, idx)

#define anv_narr_get_char(arr, idx)     anv_narr_get(arr, idx)->c
#define anv_narr_get_int(arr, idx)      anv_narr_get(arr, idx)->i
#define anv_narr_get_unsigned(arr, idx) anv_narr_get(arr, idx)->u
#define anv_narr_get_long(arr, idx)     anv_narr_get(arr, idx)->l
#define anv_narr_get_longlong(arr, idx) anv_narr_get(arr, idx)->ll
#define anv_narr_get_i8(arr, idx)       anv_narr_get(arr, idx)->i8
#define anv_narr_get_u8(arr, idx)       anv_narr_get(arr, idx)->u8
#define anv_narr_get_i16(arr, idx)      anv_narr_get(arr, idx)->i16
#define anv_narr_get_u16(arr, idx)      anv_narr_get(arr, idx)->u16
#define anv_narr_get_i32(arr, idx)      anv_narr_get(arr, idx)->i32
#define anv_narr_get_u32(arr, idx)      anv_narr_get(arr, idx)->u32
#define anv_narr_get_i64(arr, idx)      anv_narr_get(arr, idx)->i64
#define anv_narr_get_u64(arr, idx)      anv_narr_get(arr, idx)->u64
#define anv_narr_get_sz(arr, idx)       anv_narr_get(arr, idx)->sz
#define anv_narr_get_float(arr, idx)    anv_narr_get(arr, idx)->f
#define anv_narr_get_double(arr, idx)   anv_narr_get(arr, idx)->d

#define anv_narr_push(arr, numtype, numvalue)                                  \
    anv_arr_push_new(arr, anv_narr_numerical_t, { .numtype = (numvalue) })

#define anv_narr_push_char(arr, numvalue)     anv_narr_push(arr, c, numvalue)
#define anv_narr_push_int(arr, numvalue)      anv_narr_push(arr, i, numvalue)
#define anv_narr_push_unsigned(arr, numvalue) anv_narr_push(arr, u, numvalue)
#define anv_narr_push_long(arr, numvalue)     anv_narr_push(arr, l, numvalue)
#define anv_narr_push_longlong(arr, numvalue) anv_narr_push(arr, ll, numvalue)
#define anv_narr_push_i8(arr, numvalue)       anv_narr_push(arr, i8, numvalue)
#define anv_narr_push_u8(arr, numvalue)       anv_narr_push(arr, u8, numvalue)
#define anv_narr_push_i16(arr, numvalue)      anv_narr_push(arr, i16, numvalue)
#define anv_narr_push_u16(arr, numvalue)      anv_narr_push(arr, u16, numvalue)
#define anv_narr_push_i32(arr, numvalue)      anv_narr_push(arr, i32, numvalue)
#define anv_narr_push_u32(arr, numvalue)      anv_narr_push(arr, u32, numvalue)
#define anv_narr_push_i64(arr, numvalue)      anv_narr_push(arr, i64, numvalue)
#define anv_narr_push_u64(arr, numvalue)      anv_narr_push(arr, u64, numvalue)
#define anv_narr_push_sz(arr, numvalue)       anv_narr_push(arr, sz, numvalue)
#define anv_narr_push_float(arr, numvalue)    anv_narr_push(arr, f, numvalue)
#define anv_narr_push_double(arr, numvalue)   anv_narr_push(arr, d, numvalue)

#ifdef __cplusplus
}
#endif

#ifdef ANV_NARR_IMPLEMENTATION

    #ifndef anv_arr__assert
        #include <assert.h>
        #define anv_arr__assert(x) assert(x)
    #endif

    #ifdef __GNUC__
        #define ANV_ARR__LIKELY(x)   __builtin_expect((x), 1)
        #define ANV_ARR__UNLIKELY(x) __builtin_expect((x), 0)
    #else
        #define ANV_ARR__LIKELY(x)   (x)
        #define ANV_ARR__UNLIKELY(x) (x)
    #endif

#endif /* ANV_NARR_IMPLEMENTATION */

#endif /* ANV_NARR_H */
