#include "../include/anv_testsuite_2.h"

ANV_TESTSUITE_FIXTURE(tests_fail_failure)
{
    expect(0);
}

ANV_TESTSUITE_FIXTURE(tests_fail_failure_msg)
{
    expect_msg(0, "This is a failure");
}

ANV_TESTSUITE(
    tests_fail,
    ANV_TESTSUITE_REGISTER(tests_fail_failure),
    ANV_TESTSUITE_REGISTER(tests_fail_failure_msg),
);

ANV_TESTSUITE_FIXTURE(tests_successful_success)
{
    expect(1);
}

ANV_TESTSUITE_FIXTURE(tests_successful_success_msg)
{
    expect_msg(1, "This is a success");
}

ANV_TESTSUITE(
    tests_successful,
    ANV_TESTSUITE_REGISTER(tests_successful_success),
    ANV_TESTSUITE_REGISTER(tests_successful_success_msg),
);

ANV_TESTSUITE_FIXTURE(tests_mixed_failure)
{
    expect(0);
}

ANV_TESTSUITE_FIXTURE(tests_mixed_success)
{
    expect(1);
}

ANV_TESTSUITE_FIXTURE(tests_mixed_success_msg)
{
    expect_msg(1, "Success really");
}

ANV_TESTSUITE_FIXTURE(tests_mixed_failure_multi)
{
    expect(0 == 0 && 0 == 1);
}

ANV_TESTSUITE_FIXTURE(tests_mixed_failure_msg)
{
    expect_msg(0, "Welp, this failed");
}

ANV_TESTSUITE(
    tests_mixed,
    ANV_TESTSUITE_REGISTER(tests_mixed_failure),
    ANV_TESTSUITE_REGISTER(tests_mixed_success),
    ANV_TESTSUITE_REGISTER(tests_mixed_success_msg),
    ANV_TESTSUITE_REGISTER(tests_mixed_failure_multi),
    ANV_TESTSUITE_REGISTER(tests_mixed_failure_msg),
);

ANV_TESTSUITE_FIXTURE(tests_setup_teardown_success)
{
    expect(1);
}

ANV_TESTSUITE_FIXTURE(tests_setup_teardown_failure_msg)
{
    expect_msg(0, "This is a fail");
}

static int
tests_setup_teardown_setup(FILE *out_file)
{
    fprintf(out_file, "setup!\n");
    return 0;
}

static int
tests_setup_teardown_teardown(FILE *out_file)
{
    fprintf(out_file, "teardown!\n");
    return 1;
}

ANV_TESTSUITE_WITH_CONFIG(
    tests_setup_teardown,
    ANV_TESTSUITE_REGISTER(tests_setup_teardown_success),
    ANV_TESTSUITE_REGISTER(tests_setup_teardown_failure_msg),
) {
    .setup = tests_setup_teardown_setup,
    .teardown = tests_setup_teardown_teardown,
};

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
    // simple test suites
    ANV_TESTSUITE_RUN(tests_fail, stdout);
    ANV_TESTSUITE_RUN(tests_successful, stdout);
    ANV_TESTSUITE_RUN(tests_mixed, stdout);

    // test suites with hooks
    ANV_TESTSUITE_RUN(tests_setup_teardown, stdout);
    ANV_TESTSUITE_RUN(tests_callbacks, stdout);
}
