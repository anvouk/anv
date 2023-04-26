#include "../include/anv_testsuite_2.h"

#define ANV_HALLOC_IMPLEMENTATION
#include "../include/anv_halloc.h"

ANV_TESTSUITE_FIXTURE(anvh_alloc_no_parent_no_children)
{
    void *mem = anvh_alloc(NULL, 10000, 0);
    expect(mem);

    anvh_free(mem);
}

ANV_TESTSUITE_FIXTURE(anvh_alloc_1_parent_1_children)
{
    void *mem = anvh_alloc(NULL, 10000, 1);
    expect(mem);
    void *mem1 = anvh_alloc(mem, 20000, 0);
    expect(mem1);

    anvh_free(mem);
}

ANV_TESTSUITE_FIXTURE(anvh_alloc_1_parent_1_children_with_extra_space)
{
    void *mem = anvh_alloc(NULL, sizeof(int), 10);
    expect(mem);
    void *mem2 = anvh_alloc(mem, sizeof(int), 0);
    expect(mem2);

    anvh_free(mem);
}

ANV_TESTSUITE_FIXTURE(anvh_alloc_no_parent_a_lot_of_children)
{
    uint16_t children_cap = 300;
    void *mem = anvh_alloc(NULL, 5000, children_cap);
    expect(mem);
    for (uint16_t i = 0; i < children_cap; ++i) {
        void *mem1 = anvh_alloc(mem, 500, 0);
        expect(mem1);
    }

    anvh_free(mem);
}

ANV_TESTSUITE_FIXTURE(anvh_alloc_error_on_no_space_for_children)
{
    void *mem = anvh_alloc(NULL, sizeof(int), 0);
    expect(mem);
    void *mem2 = anvh_alloc(mem, sizeof(int), 0);
    expect(!mem2);

    anvh_free(mem);
}

ANV_TESTSUITE(
    tests_anv_halloc,
    ANV_TESTSUITE_REGISTER(anvh_alloc_no_parent_no_children),
    ANV_TESTSUITE_REGISTER(anvh_alloc_1_parent_1_children),
    ANV_TESTSUITE_REGISTER(anvh_alloc_1_parent_1_children_with_extra_space),
    ANV_TESTSUITE_REGISTER(anvh_alloc_no_parent_a_lot_of_children),
    ANV_TESTSUITE_REGISTER(anvh_alloc_error_on_no_space_for_children),
);

int
main(void)
{
    anv_testsuite_catch_crashes();
    ANV_TESTSUITE_RUN(tests_anv_halloc, stdout);
}
