/*
 * The MIT License
 *
 * Copyright 2019 Andrea Vouk.
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
    anv_testsuite
--------------------------------------------------------------------------------

  minimalist C/C++ unit test framework.

  main features:
  - extremely small and compact
  - ease of use
  - does the job, nothing more, nothing less
  - no hidden complicated implementations

  simple example:

    #include "anv_testsuite.h"

    ANV_TESTSUITE_FIXTURE(test_group_1_fixture_1)
    {
        int a = 10;
        expect(a == 10);
        
        int b = 13;
        expect(a == b);
    }

    ANV_TESTSUITE_FIXTURE(test_group_1_fixture_2)
    {
    }

    ANV_TESTSUITE_FIXTURE(test_group_1_fixture_3)
    {
        expect_msg(0, "rip");
    }

    ANV_TESTSUITE(test_group_1,
        ANV_TESTSUITE_ADD(test_group_1_fixture_1),
        ANV_TESTSUITE_ADD(test_group_1_fixture_2),
        ANV_TESTSUITE_ADD(test_group_1_fixture_3),
        );

    ANV_TESTSUITE_FIXTURE(test_group_2_fixture_1)
    {
    }

    ANV_TESTSUITE(test_group_2,
        ANV_TESTSUITE_ADD(test_group_2_fixture_1),
        );

    int main(void)
    {
        ANV_TESTSUITE_BEGIN(stdout);    // optional
        ANV_TESTSUITE_RUN(test_group_1, stdout);
        ANV_TESTSUITE_RUN(test_group_2, stdout);
        ANV_TESTSUITE_END(stdout);      // optional
    }

------------------------------------------------------------------------------*/

#ifndef ANV_TESTSUITE_H
#define ANV_TESTSUITE_H

#include <stdio.h>  /* for FILE, fprintf() */
#include <assert.h> /* for assert() */

#define ANV_TESTSUITE__LEN(x) (sizeof(x) / sizeof((x)[0]))

typedef struct anv_testsuite_fixture {
    const char *fixture_name;
    void(*fixture)(int *out_res, FILE *out_file);
} anv_testsuite_fixture;

#define ANV_TESTSUITE_FIXTURE(fixture) \
    void fixture(int *out_res, FILE *out_file)

#define ANV_TESTSUITE_ADD(fixture) \
    (anv_testsuite_fixture) { #fixture, fixture }

#define ANV_TESTSUITE(suitename, ...) \
    const anv_testsuite_fixture suitename[] = { \
        __VA_ARGS__ \
    }

#define ANV_TESTSUITE_BEGIN(out_file) \
    fprintf(out_file, "==== BEGIN RUNNING TESTS ====\n")

#define ANV_TESTSUITE_END(out_file) \
    fprintf(out_file, "==== END RUNNING TESTS ====\n\n")

#define ANV_TESTSUITE_RUN(suitename, out_file) \
    do { \
        int total_fails = 0; \
        fprintf(out_file, "---- BEGIN TEST SUITE: %s ----\n\n", #suitename); \
        for (int i = 0; i < ANV_TESTSUITE__LEN(suitename); ++i) { \
            fprintf(out_file, "[%d] %-30s => ", i, ((suitename)[i]).fixture_name); \
            int result = 1; \
            ((suitename)[i]).fixture(&result, out_file); \
            if (result) { \
                fprintf(out_file, "SUCCESS\n"); \
            } else { \
                fprintf(out_file, "\n"); \
                ++total_fails; \
            } \
        } \
        fprintf(out_file, "\nSUCCESS: %d/%d\n", \
            (int)ANV_TESTSUITE__LEN(suitename) - total_fails, \
            (int)ANV_TESTSUITE__LEN(suitename)); \
        fprintf(out_file, "\n---- END TEST SUITE: %s ----\n", #suitename); \
    } while (0)

#define anv_testsuite_expect(cond) \
    if (!(cond)) { \
        fprintf(out_file, "FAILURE(%d): '%s'", __LINE__, #cond); \
        *out_res = 0; \
        return; \
    }

#define anv_testsuite_expect_msg(cond, msg) \
    if (!(cond)) { \
        fprintf(out_file, "FAILURE(%d): '%s' (%s)", __LINE__, #cond, msg); \
        *out_res = 0; \
        return; \
    }

#ifndef ANV_TESTSUITE_DISABLE_ABBREVIATIONS
#  define expect        anv_testsuite_expect
#  define expect_msg    anv_testsuite_expect_msg
#endif

#endif /* ANV_TESTSUITE_H */
