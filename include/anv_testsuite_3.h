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
    anv_testsuite_2 (https://github.com/anvouk/anv)
--------------------------------------------------------------------------------

# anv_testsuite_2

Extremely simple C unit testing framework.

Main goals:
- small and compact
- portable
- ease of use
- fast
- minimal features
- optional colors

## Dependencies

None

## Include usage

```c
#include "anv_testsuite_2.h"
```

## About crashes handling:

This library optionally supports a best-effort crash detection logging and
graceful shutdown (see anv_testsuite_catch_crashes example).

Since the lib's goal is simplicity and portability, we do not go
the extra mile to get stack traces or other fancy info because they are very
much OS-specific.

The current implementation involves basic C signal() callbacks and logs to
stderr.

## Examples

### Simple example

```c
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
```

### Example with setup/teardown

```c
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
```

### Example with before/after each callbacks

```c
ANV_TESTSUITE_FIXTURE(tests_callbacks_success)
{
    expect(1);
}

ANV_TESTSUITE_FIXTURE(tests_callbacks_failure_msg)
{
    expect_msg(0, "This is a fail");
}

static void
tests_callbacks_before_each(void)
{
    printf("before_each!\n");
}

static void
tests_callbacks_after_each(void)
{
    printf("after_each!\n");
}

ANV_TESTSUITE_WITH_CONFIG(
    tests_callbacks,
    ANV_TESTSUITE_REGISTER(tests_callbacks_success),
    ANV_TESTSUITE_REGISTER(tests_callbacks_failure_msg),
) {
    .before_each = tests_callbacks_before_each,
    .after_each = tests_callbacks_after_each,
};

int
main(void)
{
    ANV_TESTSUITE_RUN(tests_callbacks, stdout);
}
```

### Example with basic crash handling

```c
ANV_TESTSUITE_FIXTURE(tests_crash_invalid_division)
{
    int b = 0;
    if (1) {
        int c = 10 / b;
        (void)c;
    }
    expect(1);
}

ANV_TESTSUITE(
    tests_crash,
    ANV_TESTSUITE_REGISTER(tests_crash_invalid_division),
);

int
main(void)
{
    // enable listeners for abnormal errors
    anv_testsuite_catch_crashes();

    ANV_TESTSUITE_RUN(tests_crash, stdout);
}
```

------------------------------------------------------------------------------*/

#ifndef ANV_TESTSUITE_3_H
#define ANV_TESTSUITE_3_H

#include <signal.h> /* for signal() */
#include <stdio.h> /* for FILE, fprintf(), snprintf() */
#include <stdlib.h> /* for exit() */
#include <string.h> /* for strlen() */

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
 * Callback function run before/after each test fixture.
 */
typedef void (*anv_testsuite_each_callback)(void);

/**
 * Custom config for test suite.
 * @note This is not a runtime config but a static one set during the test
 *       suite declaration.
 */
typedef struct anv_testsuite_config {
    anv_testsuite_setup_callback setup;
    anv_testsuite_teardown_callback teardown;
    anv_testsuite_each_callback before_each;
    anv_testsuite_each_callback after_each;
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
    static const anv_testsuite_config suitename##_config = { 0 }

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
anv_testsuite__handle_crash(int sig)
{
    fprintf(
        stderr,
        ANV_TESTSUITE__STR_RED(
            "\n******************** CRASH ********************\n"
        )
    );
    switch (sig) {
        case SIGABRT:
            fprintf(
                stderr,
                ANV_TESTSUITE__STR_RED("           REASON:        'SIGABRT'\n")
            );
            break;
        case SIGFPE:
            fprintf(
                stderr,
                ANV_TESTSUITE__STR_RED("           REASON:        'SIGFPE'\n")
            );
            break;
        case SIGILL:
            fprintf(
                stderr,
                ANV_TESTSUITE__STR_RED("           REASON:        'SIGILL'\n")
            );
            break;
        case SIGSEGV:
            fprintf(
                stderr,
                ANV_TESTSUITE__STR_RED("           REASON:        'SIGSEGV'\n")
            );
            break;
        default:
            fprintf(
                stderr,
                ANV_TESTSUITE__STR_RED("           REASON:        '%d'\n"),
                sig
            );
            break;
    }
    fprintf(
        stderr,
        ANV_TESTSUITE__STR_RED(
            "***********************************************\n"
        )
    );
    exit(1);
}

/**
 * Catches potential crashes, logs what happened and gracefully exits.
 * @note There is portable standard way to determine where the crash happened
 *       and what caused it.
 * @todo Is it worth it to implement OS specific stack trace logging?
 *       See https://gist.github.com/jvranish/4441299 for a possible example.
 *       Even so, there would be not support for envs like Cygwin and MinGW. Is
 *       the additional complexity and maintenance burden worth having a stack
 *       trace support on some OSes?
 */
void
anv_testsuite_catch_crashes(void)
{
    signal(SIGABRT, anv_testsuite__handle_crash);
    signal(SIGFPE, anv_testsuite__handle_crash);
    signal(SIGILL, anv_testsuite__handle_crash);
    signal(SIGSEGV, anv_testsuite__handle_crash);
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

        // force flush here because if test fixture crashes we may not be able
        // to get the line log.
        fflush(out_file);

        if (config->before_each) {
            config->before_each();
        }

        // run test
        int result = 0;
        suite[i].fixture(&result, out_file);

        if (config->after_each) {
            config->after_each();
        }

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

static void
anv_testsuite__expect_failed(
    const char *filename,
    int line,
    FILE *out_file,
    const char *cond_str,
    const char *msg
)
{
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
}

/**
 * Assert if condition is valid.
 */
#define anv_testsuite_expect(cond)                                             \
    do {                                                                       \
        if (!(cond)) {                                                         \
            anv_testsuite__expect_failed(                                      \
                __FILE__, __LINE__, out_file, #cond, NULL                      \
            );                                                                 \
            *out_res = 1;                                                      \
            return;                                                            \
        }                                                                      \
    } while (0)

/**
 * Assert if condition is valid and print custom msg on failure.
 */
#define anv_testsuite_expect_msg(cond, msg)                                    \
    do {                                                                       \
        if (!(cond)) {                                                         \
            anv_testsuite__expect_failed(                                      \
                __FILE__, __LINE__, out_file, #cond, msg                       \
            );                                                                 \
            *out_res = 1;                                                      \
            return;                                                            \
        }                                                                      \
    } while (0)

#ifndef ANV_TESTSUITE_DISABLE_ABBREVIATIONS
    #define expect     anv_testsuite_expect
    #define expect_msg anv_testsuite_expect_msg
#endif

#endif /* ANV_TESTSUITE_3_H */
