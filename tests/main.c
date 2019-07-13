#include <stdio.h>

#include "tests_anv_leaks.h"
#include "tests_anv_metalloc.h"

int
main(void)
{
    ANV_TESTSUITE_BEGIN(stdout);
    ANV_TESTSUITE_RUN(tests_anv_leaks,      stdout);
    ANV_TESTSUITE_RUN(tests_anv_metalloc,   stdout);
    ANV_TESTSUITE_END(stdout);
}
