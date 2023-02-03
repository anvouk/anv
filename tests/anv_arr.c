#include "../include/anv_testsuite_2.h"

#define ANV_METALLOC_IMPLEMENTATION
#define anv_meta__assert(cond, errmsg) ((void)(cond))
#define ANV_ARR_IMPLEMENTATION
#define anv_arr__assert(cond, errmsg) ((void)(cond))
#include "../include/anv_arr.h"

typedef struct item_t {
    int a;
} item_t;

ANV_TESTSUITE_FIXTURE(anv_arr_new_with_capacity_0_is_null)
{
    anv_arr_t arr = anv_arr_new(0, sizeof(item_t));
    expect(!arr);
}

ANV_TESTSUITE_FIXTURE(anv_arr_new_with_capacity_1234_is_ok)
{
    anv_arr_t arr = anv_arr_new(1234, sizeof(item_t));
    expect(arr);
    anv_arr_destroy(arr);
}

ANV_TESTSUITE_FIXTURE(anv_arr_new_with_invalid_alloc_is_null)
{
    anv_arr_t arr = anv_arr_new(10, 0);
    expect(!arr);
    anv_arr_destroy(arr);
}

ANV_TESTSUITE_FIXTURE(anv_arr_destroy_with_null_arr_does_nothing)
{
    anv_arr_destroy(NULL);
    expect(1);
}

ANV_TESTSUITE_FIXTURE(anv_arr_destroy_with_invalid_arr_does_nothing)
{
    void *mem = malloc(10);
    expect(mem);

    anv_arr_destroy(mem);

    free(mem);
}

ANV_TESTSUITE_FIXTURE(anv_arr_length_with_0_elements_returns_0)
{
    anv_arr_t arr = anv_arr_new(10, sizeof(item_t));
    expect(arr);

    expect(anv_arr_length(arr) == 0);

    anv_arr_destroy(arr);
}

ANV_TESTSUITE_FIXTURE(anv_arr_length_with_n_elements_returns_n)
{
    anv_arr_t arr = anv_arr_new(10, sizeof(item_t));
    expect(arr);

    item_t i1 = { .a = 10 };
    expect(anv_arr_push(arr, &i1) == ANV_ARR_RESULT_OK);
    item_t i2 = { .a = 20 };
    expect(anv_arr_push(arr, &i2) == ANV_ARR_RESULT_OK);

    expect(anv_arr_length(arr) == 2);

    anv_arr_destroy(arr);
}

ANV_TESTSUITE_FIXTURE(anv_arr_push_with_1_element_under_capacity_is_ok)
{
    anv_arr_t arr = anv_arr_new(10, sizeof(item_t));
    expect(arr);

    item_t item = { .a = 100 };
    expect(anv_arr_push(arr, &item) == ANV_ARR_RESULT_OK);

    anv_arr_destroy(arr);
}

ANV_TESTSUITE_FIXTURE(anv_arr_push_with_null_arr_is_param_error)
{
    anv_arr_t *ptr = NULL;

    item_t item = { .a = 100 };
    expect(anv_arr_push(ptr, &item) == ANV_ARR_RESULT_INVALID_PARAMS);
}

ANV_TESTSUITE_FIXTURE(anv_arr_push_with_null_item_is_param_error)
{
    anv_arr_t arr = anv_arr_new(10, sizeof(item_t));
    expect(arr);

    expect(anv_arr_push(arr, NULL) == ANV_ARR_RESULT_INVALID_PARAMS);

    anv_arr_destroy(arr);
}

ANV_TESTSUITE_FIXTURE(anv_arr_push_with_invalid_arr_is_param_error)
{
    void *mem = malloc(10);
    expect(mem);

    item_t item = { .a = 100 };
    expect(anv_arr_push(mem, &item) == ANV_ARR_RESULT_INVALID_PARAMS);

    free(mem);
}

ANV_TESTSUITE_FIXTURE(anv_arr_push_with_arr_low_capacity_is_extended)
{
    anv_arr_t arr = anv_arr_new(1, sizeof(item_t));
    expect(arr);

    item_t item1 = { .a = 10 };
    expect(anv_arr_push(arr, &item1) == ANV_ARR_RESULT_OK);
    item_t item2 = { .a = 20 };
    expect(anv_arr_push(arr, &item2) == ANV_ARR_RESULT_OK);
    item_t item3 = { .a = 30 };
    expect(anv_arr_push(arr, &item3) == ANV_ARR_RESULT_OK);

    expect(anv_arr_length(arr) == 3);

    anv_arr_destroy(arr);
}

ANV_TESTSUITE_FIXTURE(anv_arr_push_new_with_new_struct_is_ok)
{
    anv_arr_t arr = anv_arr_new(10, sizeof(item_t));
    expect(arr);

    expect(anv_arr_push_new(arr, item_t, { .a = 10 }) == ANV_ARR_RESULT_OK);
    expect(anv_arr_push_new(arr, item_t, { .a = 20 }) == ANV_ARR_RESULT_OK);
    expect(anv_arr_push_new(arr, item_t, { .a = 30 }) == ANV_ARR_RESULT_OK);

    expect(anv_arr_length(arr) == 3);

    anv_arr_destroy(arr);
}

static size_t
custom_reallocator(size_t old_capacity)
{
    return old_capacity + 1;
}

ANV_TESTSUITE_FIXTURE(anv_arr_config_reallocator_fn_custom_ok)
{
    anv_arr_config_reallocator_fn(custom_reallocator);
    anv_arr_t arr = anv_arr_new(1, sizeof(item_t));
    expect(arr);

    item_t item1 = { .a = 10 };
    expect(anv_arr_push(arr, &item1) == ANV_ARR_RESULT_OK);
    item_t item2 = { .a = 20 };
    expect(anv_arr_push(arr, &item2) == ANV_ARR_RESULT_OK);
    item_t item3 = { .a = 30 };
    expect(anv_arr_push(arr, &item3) == ANV_ARR_RESULT_OK);

    expect(anv_arr_length(arr) == 3);

    anv_arr_destroy(arr);
}

ANV_TESTSUITE_FIXTURE(anv_arr_config_reallocator_fn_restore_default_ok)
{
    anv_arr_config_reallocator_fn(custom_reallocator);
    anv_arr_config_reallocator_fn(NULL);
    anv_arr_t arr = anv_arr_new(1, sizeof(item_t));
    expect(arr);

    item_t item1 = { .a = 10 };
    expect(anv_arr_push(arr, &item1) == ANV_ARR_RESULT_OK);
    item_t item2 = { .a = 20 };
    expect(anv_arr_push(arr, &item2) == ANV_ARR_RESULT_OK);
    item_t item3 = { .a = 30 };
    expect(anv_arr_push(arr, &item3) == ANV_ARR_RESULT_OK);

    expect(anv_arr_length(arr) == 3);

    anv_arr_destroy(arr);
}

ANV_TESTSUITE_FIXTURE(anv_arr_get_with_no_elements_returns_null)
{
    anv_arr_t arr = anv_arr_new(10, sizeof(item_t));
    expect(arr);

    expect(anv_arr_get(arr, item_t, 0) == NULL);

    anv_arr_destroy(arr);
}

ANV_TESTSUITE_FIXTURE(anv_arr_get_with_null_arr_returns_null)
{
    expect(anv_arr_get(NULL, item_t, 0) == NULL);
}

ANV_TESTSUITE_FIXTURE(anv_arr_get_with_invalid_arr_returns_null)
{
    void *mem = malloc(10);
    expect(mem);

    expect(anv_arr_get(mem, item_t, 0) == NULL);

    free(mem);
}

ANV_TESTSUITE_FIXTURE(anv_arr_get_with_element_returns_correct_element)
{
    anv_arr_t arr = anv_arr_new(10, sizeof(item_t));
    expect(arr);

    item_t item1 = { .a = 69 };
    expect(anv_arr_push(arr, &item1) == ANV_ARR_RESULT_OK);
    item_t item2 = { .a = 690 };
    expect(anv_arr_push(arr, &item2) == ANV_ARR_RESULT_OK);
    item_t item3 = { .a = 6900 };
    expect(anv_arr_push(arr, &item3) == ANV_ARR_RESULT_OK);

    expect(anv_arr_get(arr, item_t, 1) != NULL);
    expect(anv_arr_get(arr, item_t, 1)->a == 690);

    anv_arr_destroy(arr);
}

ANV_TESTSUITE_FIXTURE(anv_arr_for_loop_with_no_elements_is_ok)
{
    anv_arr_t arr = anv_arr_new(10, sizeof(item_t));
    expect(arr);

    size_t arr_len = anv_arr_length(arr);
    expect(arr_len == 0);

    for (size_t i = 0; i < arr_len; ++i) {
        item_t *item = anv_arr_get(arr, item_t, i);
        expect(item != NULL);
    }

    anv_arr_destroy(arr);
}

ANV_TESTSUITE_FIXTURE(anv_arr_for_loop_with_elements_is_ok)
{
    anv_arr_t arr = anv_arr_new(10, sizeof(item_t));
    expect(arr);

    item_t item1 = { .a = 69 };
    expect(anv_arr_push(arr, &item1) == ANV_ARR_RESULT_OK);
    item_t item2 = { .a = 70 };
    expect(anv_arr_push(arr, &item2) == ANV_ARR_RESULT_OK);
    item_t item3 = { .a = 71 };
    expect(anv_arr_push(arr, &item3) == ANV_ARR_RESULT_OK);

    size_t arr_len = anv_arr_length(arr);
    expect(arr_len == 3);

    for (size_t i = 0; i < arr_len; ++i) {
        item_t *item = anv_arr_get(arr, item_t, i);
        expect(item != NULL);
        expect((size_t)item->a == 69 + i);
    }

    anv_arr_destroy(arr);
}

ANV_TESTSUITE(
    tests_anv_arr,
    ANV_TESTSUITE_REGISTER(anv_arr_new_with_capacity_0_is_null),
    ANV_TESTSUITE_REGISTER(anv_arr_new_with_capacity_1234_is_ok),
    ANV_TESTSUITE_REGISTER(anv_arr_new_with_invalid_alloc_is_null),
    ANV_TESTSUITE_REGISTER(anv_arr_destroy_with_null_arr_does_nothing),
    ANV_TESTSUITE_REGISTER(anv_arr_destroy_with_invalid_arr_does_nothing),
    ANV_TESTSUITE_REGISTER(anv_arr_length_with_0_elements_returns_0),
    ANV_TESTSUITE_REGISTER(anv_arr_length_with_n_elements_returns_n),
    ANV_TESTSUITE_REGISTER(anv_arr_push_with_1_element_under_capacity_is_ok),
    ANV_TESTSUITE_REGISTER(anv_arr_push_with_null_arr_is_param_error),
    ANV_TESTSUITE_REGISTER(anv_arr_push_with_null_item_is_param_error),
    ANV_TESTSUITE_REGISTER(anv_arr_push_with_invalid_arr_is_param_error),
    ANV_TESTSUITE_REGISTER(anv_arr_push_with_arr_low_capacity_is_extended),
    ANV_TESTSUITE_REGISTER(anv_arr_push_new_with_new_struct_is_ok),
    ANV_TESTSUITE_REGISTER(anv_arr_config_reallocator_fn_custom_ok),
    ANV_TESTSUITE_REGISTER(anv_arr_config_reallocator_fn_restore_default_ok),
    ANV_TESTSUITE_REGISTER(anv_arr_get_with_no_elements_returns_null),
    ANV_TESTSUITE_REGISTER(anv_arr_get_with_null_arr_returns_null),
    ANV_TESTSUITE_REGISTER(anv_arr_get_with_invalid_arr_returns_null),
    ANV_TESTSUITE_REGISTER(anv_arr_get_with_element_returns_correct_element),
    ANV_TESTSUITE_REGISTER(anv_arr_for_loop_with_no_elements_is_ok),
    ANV_TESTSUITE_REGISTER(anv_arr_for_loop_with_elements_is_ok),
);

int
main(void)
{
    anv_testsuite_catch_crashes();
    ANV_TESTSUITE_RUN(tests_anv_arr, stdout);
}
