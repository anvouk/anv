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
    anv_bench
--------------------------------------------------------------------------------

  quick-and-dirty benchmarking system for quick prototyping.

  simple example:

    #include <anv_bench.h>

    ANV_BENCH_NOINLINE void test_func(int *a, int b, int c)
    {
        *a += (b - c) * 2 + *a;
    }

    int main(void)
    {
        int a = 0;
        ANV_BENCH_QUICK(test_func, &a, 2, 3);
    }

  use ANV_BENCH_WITH_NAME or ANV_BENCH to get more control over the benchmarks.

  groups benchmarking example:

    #include <anv_bench.h>

    ANV_BENCH_NOINLINE void test_func_a(int *a, int b, int c)
    {
        *a += (b - c) * 2 + *a;
    }

    ANV_BENCH_NOINLINE int test_func_b(int b, int c)
    {
        return (b + c) / 2;
    }

    int main(void)
    {
        int a = 0;

        ANV_BENCH_BEGIN(stdout, 10, 1000);
            ANV_BENCH_ADD(test_func_a, &a, 2, 3);
            ANV_BENCH_ADD(test_func_b, 2, 3);
        ANV_BENCH_END();
    }

------------------------------------------------------------------------------*/

#ifndef ANV_BENCH_H
#define ANV_BENCH_H

#include <stdint.h>
#include <stdio.h>

/* note: using va_args is a nice hack for the MSVC warning
 * "warning C4003: not enough actual parameters for macro '_QBSTR'" that
 * happens when trying to benchmark a function without parameters.
 */

#define _ANVSTR_VA(...) #__VA_ARGS__
#define ANVSTR_VA       _ANVSTR_VA

#ifdef _MSC_VER
    #define ANV_BENCH_NOINLINE __declspec(noinline)
#else
    #define ANV_BENCH_NOINLINE __attribute__((noinline))
#endif /* _MSC_VER */

#ifndef ANV_BENCH_FUNC
    #ifdef _MSC_VER
        #define ANV_BENCH_FUNC __rdtsc
    #else
        #define ANV_BENCH_FUNC __builtin_ia32_rdtsc
    #endif
#endif

/*------------------------------------------------------------------------------
    single bench
------------------------------------------------------------------------------*/

#define ANV_BENCH_WITH_NAME(file, n_runs, name, func, ...)                     \
    do {                                                                       \
        uint64_t _tot = 0;                                                     \
        volatile int _j;                                                       \
        for (_j = 0; _j < n_runs; ++_j) {                                      \
            uint64_t _start, _end;                                             \
            _start = (uint64_t)ANV_BENCH_FUNC();                               \
            func(__VA_ARGS__);                                                 \
            _end = (uint64_t)ANV_BENCH_FUNC();                                 \
            _tot += _end - _start;                                             \
        }                                                                      \
        fprintf(                                                               \
            file,                                                              \
            "%-50s calls: %.4d %10s %lld\n",                                   \
            name,                                                              \
            (int)n_runs,                                                       \
            "value:",                                                          \
            _tot / n_runs                                                      \
        );                                                                     \
    } while (0)

#define ANV_BENCH(file, n_runs, func, ...)                                     \
    ANV_BENCH_WITH_NAME(                                                       \
        file,                                                                  \
        n_runs,                                                                \
        ANVSTR_VA(func) "(" ANVSTR_VA(__VA_ARGS__) ")",                        \
        func,                                                                  \
        __VA_ARGS__                                                            \
    )

#define ANV_BENCH_QUICK(func, ...) ANV_BENCH(stdout, 1000, func, __VA_ARGS__)

/*------------------------------------------------------------------------------
    groups bench
------------------------------------------------------------------------------*/

#define ANV_BENCH_BEGIN(file, g_runs, n_runs)                                  \
    volatile int _i;                                                           \
    for (_i = 0; _i < (g_runs); ++_i) {                                        \
        FILE *_outfile = file;                                                 \
        uint64_t _single_runs = (n_runs);                                      \
        fprintf(                                                               \
            file,                                                              \
            "============================================================"     \
            "========================= n. %.2d\n",                             \
            _i + 1                                                             \
        )

#define ANV_BENCH_ADD(func, ...)                                               \
    ANV_BENCH(_outfile, _single_runs, func, __VA_ARGS__)

#define ANV_BENCH_ADD_WITH_NAME(name, func, ...)                               \
    ANV_BENCH_WITH_NAME(_outfile, _single_runs, name, func, __VA_ARGS__)

#define ANV_BENCH_END()                                                        \
    }                                                                          \
    ((void)0) /* for the ; */

#endif /* ANV_BENCH_H */
