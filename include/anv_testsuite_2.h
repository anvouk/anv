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
    anv_testsuite_2
--------------------------------------------------------------------------------

  extremely simple C unit test framework.

  main goals:
  - small and compact
  - ease of use
  - fast
  - minimal features
  - optional colors

  simple example:

    #include "anv_testsuite_2.h"

    ANV_TESTSUITE_FIXTURE(demo_success)
    {
        expect(1);
    }

    ANV_TESTSUITE_FIXTURE(demo_failure_with_msg)
    {
        expect_msg(1 == 0, "Ooops");
    }

    ANV_TESTSUITE(
        my_testsuite,
        ANV_TESTSUITE_REGISTER(demo_success),
        ANV_TESTSUITE_REGISTER(demo_failure_with_msg),
    );

    int
    main(void)
    {
        ANV_TESTSUITE_RUN(my_testsuite, stdout);
    }

  example with setup/teardown:

    ANV_TESTSUITE_FIXTURE(tests_config_success)
    {
        expect(1);
    }

    static int
    tests_config_setup(FILE *out_file)
    {
        fprintf(out_file, "setup!\n");
        return 0;
    }

    static int
    tests_config_teardown(FILE *out_file)
    {
        fprintf(out_file, "teardown!\n");
        return 1; // signal teardown failed
    }

    ANV_TESTSUITE_WITH_CONFIG(
        tests_config,
        ANV_TESTSUITE_REGISTER(tests_config_success),
    ) {
        .setup = tests_config_setup,
        .teardown = tests_config_teardown,
    };

    int
    main(void)
    {
        ANV_TESTSUITE_RUN(tests_config, stdout);
    }

------------------------------------------------------------------------------*/

#ifndef ANV_TESTSUITE_2_H
#define ANV_TESTSUITE_2_H

#include <stdio.h> /* for FILE, fprintf(), snprintf */
#include <string.h> /* for strlen */

/**
 * Toggle UNIX console colors.
 */
#ifdef ANV_TESTSUITE_DISABLE_COLORS
    #define ANV_TESTSUITE__STR_GREEN(str) str
    #define ANV_TESTSUITE__STR_RED(str)   str
#else
    #define ANV_TESTSUITE__STR_GREEN(str) "\033[32m" str "\033[39m"
    #define ANV_TESTSUITE__STR_RED(str)   "\033[31m" str "\033[39m"
#endif

/**
 * Change dotted padding length for fixtures.
 */
#ifndef ANV_TESTSUITE_PADDING
    #define ANV_TESTSUITE_PADDING 100
#endif

#define ANV_TESTSUITE__LEN(x) (sizeof(x) / sizeof((x)[0]))

#define ANV_TESTSUITE__PADDING                                                 \
    "........................................................................" \
    "......................"

/**
 * Always run before running the first fixture.
 * If this function fails, the test suite is aborted.
 * @param out_file File to where the test suite output is being directed to.
 * @return 0 on success.
 */
typedef int (*anv_testsuite_setup_callback)(FILE *out_file);

/**
 * Always run after running the last fixture.
 * If this function fails, the test will continue as normal.
 * @param out_file File to where the test suite output is being directed to.
 * @return 0 on success.
 */
typedef int (*anv_testsuite_teardown_callback)(FILE *out_file);

/**
 * Custom config for test suite.
 * @note This is not a runtime config but a static one set during the test
 *       suite declaration.
 */
typedef struct anv_testsuite_config {
    anv_testsuite_setup_callback setup;
    anv_testsuite_teardown_callback teardown;
} anv_testsuite_config;

/**
 * Test fixture interface.
 * @param out_res Must be set to non-zero values to signal errors.
 * @param out_file Output messages destination.
 */
typedef void (*anv_testsuite_fixture_callback)(int *out_res, FILE *out_file);

/**
 * Identifies a test fixture which is run by a test suite.
 */
typedef struct anv_testsuite_fixture {
    /** Display name for fixture. */
    const char *fixture_name;
    /** Fixture to run. */
    anv_testsuite_fixture_callback fixture;
} anv_testsuite_fixture;

/**
 * Define a test fixture scoped to current file only.
 * Name must be unique for current file.
 */
#define ANV_TESTSUITE_FIXTURE(fixture_name)                                    \
    static void fixture_name(int *out_res, FILE *out_file)

/**
 * Define a test suite with a list of test fixtures to run scoped to current
 * file only.
 * Name must be unique for current file.
 */
#define ANV_TESTSUITE(suitename, ...)                                          \
    static const anv_testsuite_fixture suitename[] = { __VA_ARGS__ };          \
    static const anv_testsuite_config suitename##_config = { NULL, NULL }

/**
 * Define a test suite with a list of test fixtures to run scoped to current
 * file only and a custom configuration.
 * Name must be unique for current file.
 */
#define ANV_TESTSUITE_WITH_CONFIG(suitename, ...)                              \
    static const anv_testsuite_fixture suitename[] = { __VA_ARGS__ };          \
    static const anv_testsuite_config suitename##_config =

/**
 * Register test fixture for test suite.
 */
#define ANV_TESTSUITE_REGISTER(fixture_name)                                   \
    {                                                                          \
        #fixture_name, fixture_name                                            \
    }

static void
anv_testsuite__run(
    const char *filename,
    int line,
    const anv_testsuite_fixture *suite,
    size_t suite_sz,
    const char *suitename,
    FILE *out_file,
    const anv_testsuite_config *config
)
{
    int total_fails = 0;
    fprintf(out_file, "Suite(%s:%d): %s\n", filename, line, suitename);

    // run setup if present
    if (config->setup) {
        fprintf(out_file, "\nRunning setup ...\n");
        if (config->setup(out_file) == 0) {
            fprintf(
                out_file,
                "Running setup ... " ANV_TESTSUITE__STR_GREEN("SUCCESS\n\n")
            );
        } else {
            fprintf(
                out_file,
                "Running setup ... " ANV_TESTSUITE__STR_RED("FAILURE\n\n")
            );
            return;
        }
    }

    char buff[128] = { 0 };
    char padd_buff[128] = { 0 };

    for (size_t i = 0; i < suite_sz; ++i) {
        // pretty print info
        snprintf(buff, 128, "  [%03ld]  %s", i, suite[i].fixture_name);
        int padd_sz = ANV_TESTSUITE_PADDING - strlen(buff);
        if (padd_sz > 0) {
            snprintf(padd_buff, (size_t)padd_sz, "%s", ANV_TESTSUITE__PADDING);
        } else {
            padd_buff[0] = '\0';
        }
        fprintf(out_file, "%s %s ", buff, padd_buff);

        // run test
        int result = 0;
        suite[i].fixture(&result, out_file);

        if (result == 0) {
            fprintf(out_file, ANV_TESTSUITE__STR_GREEN("SUCCESS\n"));
        } else {
            ++total_fails;
        }
    }

    // run teardown cleanup if present
    if (config->teardown) {
        fprintf(out_file, "\nRunning teardown ...\n");
        if (config->teardown(out_file) == 0) {
            fprintf(
                out_file,
                "Running teardown ... " ANV_TESTSUITE__STR_GREEN("SUCCESS\n\n")
            );
        } else {
            fprintf(
                out_file,
                "Running teardown ... " ANV_TESTSUITE__STR_RED("FAILURE\n\n")
            );
        }
    }

    // testsuite results summary
    if (total_fails == 0) {
        fprintf(
            out_file,
            "Results: " ANV_TESTSUITE__STR_GREEN("%d/%d\n"),
            (int)suite_sz - total_fails,
            (int)suite_sz
        );
    } else {
        fprintf(
            out_file,
            "Results: " ANV_TESTSUITE__STR_RED("%d/%d\n"),
            (int)suite_sz - total_fails,
            (int)suite_sz
        );
    }
}

/**
 * Run all registered tests fot this testsuite and print results to file.
 */
#define ANV_TESTSUITE_RUN(suitename, out_file)                                 \
    anv_testsuite__run(                                                        \
        __FILE__,                                                              \
        __LINE__,                                                              \
        suitename,                                                             \
        ANV_TESTSUITE__LEN(suitename),                                         \
        #suitename,                                                            \
        out_file,                                                              \
        &suitename##_config                                                    \
    )

static int
anv_testsuite__expect(
    const char *filename,
    int line,
    FILE *out_file,
    int cond,
    const char *cond_str,
    const char *msg
)
{
    if (cond) {
        return 0;
    }

    fprintf(out_file, ANV_TESTSUITE__STR_RED("FAILURE\n"));
    fprintf(
        out_file,
        ANV_TESTSUITE__STR_RED("           LOCATION:      '%s:%d'\n"),
        filename,
        line
    );
    fprintf(
        out_file,
        ANV_TESTSUITE__STR_RED("           CONDITION:     '%s'\n"),
        cond_str
    );
    if (msg) {
        fprintf(
            out_file,
            ANV_TESTSUITE__STR_RED("           ERROR MESSAGE: '%s'\n"),
            msg
        );
    }
    return 1;
}

/**
 * Assert if condition is valid.
 */
#define anv_testsuite_expect(cond)                                             \
    do {                                                                       \
        int __anv_result = anv_testsuite__expect(                              \
            __FILE__, __LINE__, out_file, cond, #cond, NULL                    \
        );                                                                     \
        if (__anv_result != 0) {                                               \
            *out_res = __anv_result;                                           \
            return;                                                            \
        }                                                                      \
    } while (0)

/**
 * Assert if condition is valid and print custom msg on failure.
 */
#define anv_testsuite_expect_msg(cond, msg)                                    \
    do {                                                                       \
        int __anv_result = anv_testsuite__expect(                              \
            __FILE__, __LINE__, out_file, cond, #cond, msg                     \
        );                                                                     \
        if (__anv_result != 0) {                                               \
            *out_res = __anv_result;                                           \
            return;                                                            \
        }                                                                      \
    } while (0)

#ifndef ANV_TESTSUITE_DISABLE_ABBREVIATIONS
    #define expect     anv_testsuite_expect
    #define expect_msg anv_testsuite_expect_msg
#endif

#endif /* ANV_TESTSUITE_2_H */
