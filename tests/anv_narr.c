#include "../include/anv_testsuite_2.h"

#define ANV_METALLOC_IMPLEMENTATION
#define anv_meta__assert(cond, errmsg) ((void)(cond))
#define ANV_ARR_IMPLEMENTATION
#define anv_arr__assert(cond, errmsg) ((void)(cond))
#define ANV_NARR_IMPLEMENTATION
#define anv_narr__assert(cond, errmsg) ((void)(cond))
#include "../include/anv_narr.h"

ANV_TESTSUITE_FIXTURE(test_narr)
{
    printf("sizeof: %ld\n", sizeof(long double));

    anv_narr_t arr = anv_narr_new(10);
    expect(arr);

    expect(anv_narr_push_int(arr, 100) == ANV_ARR_RESULT_OK);
    expect(anv_narr_push_int(arr, 200) == ANV_ARR_RESULT_OK);
    expect(anv_narr_push_int(arr, 300) == ANV_ARR_RESULT_OK);
    expect(anv_narr_push_int(arr, 400) == ANV_ARR_RESULT_OK);

    printf("narr at idx[%ld]: %d\n", 2, anv_narr_get_int(arr, 2));

    size_t arr_len = anv_arr_length(arr);
    for (size_t i = 0; i < arr_len; ++i) {
        int val = anv_narr_get_int(arr, i);
        printf("int[%ld]: %d\n", i, val);
    }

    anv_narr_destroy(arr);
}

ANV_TESTSUITE(tests_anv_arr, ANV_TESTSUITE_REGISTER(test_narr), );

int
main(void)
{
    anv_testsuite_catch_crashes();
    ANV_TESTSUITE_RUN(tests_anv_arr, stdout);
}
