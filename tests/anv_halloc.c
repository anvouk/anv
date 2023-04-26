#include "../include/anv_testsuite_2.h"

#define ANV_HALLOC_IMPLEMENTATION
#include "../include/anv_halloc.h"

ANV_TESTSUITE_FIXTURE(demo)
{
    void *mem = anvh_alloc(NULL, sizeof(int), 10);
    expect(mem);
    void *mem2 = anvh_alloc(mem, sizeof(int), 0);
    expect(mem2);

    anvh_free(mem);
}

ANV_TESTSUITE(tests_anv_halloc, ANV_TESTSUITE_REGISTER(demo));

int
main(void)
{
    anv_testsuite_catch_crashes();
    ANV_TESTSUITE_RUN(tests_anv_halloc, stdout);
}
