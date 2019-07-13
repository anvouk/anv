#include "tests_anv_leaks.h"

int
main(void)
{
    ANV_TESTSUITE_BEGIN(stdout);
    ANV_TESTSUITE_RUN(tests_anv_leaks, stdout);
    ANV_TESTSUITE_END(stdout);
}
