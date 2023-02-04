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

ANV_TESTSUITE_FIXTURE(anv_arr_insert_with_null_arr_is_param_error)
{
    anv_arr_t *ptr = NULL;

    item_t item = { .a = 100 };
    expect(anv_arr_insert(ptr, 0, &item) == ANV_ARR_RESULT_INVALID_PARAMS);
}

ANV_TESTSUITE_FIXTURE(anv_arr_insert_with_invalid_arr_is_param_error)
{
    void *mem = malloc(10);
    expect(mem);

    item_t item = { .a = 100 };
    expect(anv_arr_insert(mem, 0, &item) == ANV_ARR_RESULT_INVALID_PARAMS);

    free(mem);
}

ANV_TESTSUITE_FIXTURE(anv_arr_insert_with_null_item_is_ok)
{
    anv_arr_t arr = anv_arr_new(10, sizeof(item_t));
    expect(arr);

    expect(anv_arr_insert(arr, 0, NULL) == ANV_ARR_RESULT_OK);
    item_t *found_item_0 = anv_arr_get(arr, item_t, 0);
    expect(found_item_0);
    expect(found_item_0->a == 0);

    anv_arr_destroy(arr);
}

ANV_TESTSUITE_FIXTURE(anv_arr_insert_with_out_of_bounds_index_is_error)
{
    anv_arr_t arr = anv_arr_new(10, sizeof(item_t));
    expect(arr);

    item_t item1 = { .a = 100 };
    expect(anv_arr_push(arr, &item1) == ANV_ARR_RESULT_OK);
    item_t item2 = { .a = 200 };
    expect(anv_arr_push(arr, &item2) == ANV_ARR_RESULT_OK);

    item_t item3 = { .a = 300 };
    expect(
        anv_arr_insert(arr, 2, &item3) == ANV_ARR_RESULT_INDEX_OUT_OF_BOUNDS
    );

    anv_arr_destroy(arr);
}

ANV_TESTSUITE_FIXTURE(anv_arr_insert_with_empty_array_is_ok)
{
    anv_arr_t arr = anv_arr_new(10, sizeof(item_t));
    expect(arr);

    item_t item = { .a = 100 };
    expect(anv_arr_insert(arr, 0, &item) == ANV_ARR_RESULT_OK);

    expect(anv_arr_length(arr) == 1);

    anv_arr_destroy(arr);
}

ANV_TESTSUITE_FIXTURE(anv_arr_insert_with_not_empty_array_is_ok)
{
    anv_arr_t arr = anv_arr_new(10, sizeof(item_t));
    expect(arr);

    item_t item1 = { .a = 100 };
    expect(anv_arr_push(arr, &item1) == ANV_ARR_RESULT_OK);
    item_t item2 = { .a = 200 };
    expect(anv_arr_push(arr, &item2) == ANV_ARR_RESULT_OK);

    expect(anv_arr_length(arr) == 2);
    item_t item3 = { .a = 300 };
    expect(anv_arr_insert(arr, 1, &item3) == ANV_ARR_RESULT_OK);

    expect(anv_arr_length(arr) == 3);
    item_t *found_item_new = anv_arr_get(arr, item_t, 1);
    expect(found_item_new);
    expect(found_item_new->a == 300);
    item_t *found_item_moved = anv_arr_get(arr, item_t, 2);
    expect(found_item_moved);
    expect(found_item_moved->a == 200);

    anv_arr_destroy(arr);
}

ANV_TESTSUITE_FIXTURE(anv_arr_insert_multiple_check_ordering_ok)
{
    anv_arr_t arr = anv_arr_new(10, sizeof(item_t));
    expect(arr);

    item_t item1 = { .a = 100 };
    expect(anv_arr_insert(arr, 0, &item1) == ANV_ARR_RESULT_OK);
    item_t item2 = { .a = 200 };
    expect(anv_arr_insert(arr, 0, &item2) == ANV_ARR_RESULT_OK);
    item_t item3 = { .a = 300 };
    expect(anv_arr_insert(arr, 0, &item3) == ANV_ARR_RESULT_OK);

    expect(anv_arr_length(arr) == 3);
    item_t *found_item_0 = anv_arr_get(arr, item_t, 0);
    expect(found_item_0);
    expect(found_item_0->a == 300);
    item_t *found_item_1 = anv_arr_get(arr, item_t, 1);
    expect(found_item_1);
    expect(found_item_1->a == 100);
    item_t *found_item_2 = anv_arr_get(arr, item_t, 2);
    expect(found_item_2);
    expect(found_item_2->a == 200);

    anv_arr_destroy(arr);
}

ANV_TESTSUITE_FIXTURE(anv_arr_insert_with_array_expansion_ok)
{
    anv_arr_t arr = anv_arr_new(1, sizeof(item_t));
    expect(arr);

    item_t item1 = { .a = 100 };
    expect(anv_arr_insert(arr, 0, &item1) == ANV_ARR_RESULT_OK);
    item_t item2 = { .a = 200 };
    expect(anv_arr_insert(arr, 0, &item2) == ANV_ARR_RESULT_OK);
    item_t item3 = { .a = 300 };
    expect(anv_arr_insert(arr, 0, &item3) == ANV_ARR_RESULT_OK);

    expect(anv_arr_length(arr) == 3);

    anv_arr_destroy(arr);
}

ANV_TESTSUITE_FIXTURE(anv_arr_insert_new_ok)
{
    anv_arr_t arr = anv_arr_new(10, sizeof(item_t));
    expect(arr);

    expect(
        anv_arr_insert_new(arr, 0, item_t, { .a = 100 }) == ANV_ARR_RESULT_OK
    );

    expect(anv_arr_length(arr) == 1);

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

ANV_TESTSUITE_FIXTURE(anv_arr_push_with_null_item_is_ok)
{
    anv_arr_t arr = anv_arr_new(10, sizeof(item_t));
    expect(arr);

    expect(anv_arr_push(arr, NULL) == ANV_ARR_RESULT_OK);
    item_t *found_item_0 = anv_arr_get(arr, item_t, 0);
    expect(found_item_0);
    expect(found_item_0->a == 0);

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

ANV_TESTSUITE_FIXTURE(anv_arr_pop_when_null_array_return_null)
{
    expect(anv_arr_pop(NULL, item_t) == NULL);
    expect(1);
}

ANV_TESTSUITE_FIXTURE(anv_arr_pop_when_invalid_array_return_null)
{
    void *mem = malloc(10);
    expect(mem);

    expect(anv_arr_pop(mem, item_t) == NULL);

    free(mem);
}

ANV_TESTSUITE_FIXTURE(anv_arr_pop_when_empty_array_return_null)
{
    anv_arr_t arr = anv_arr_new(10, sizeof(item_t));
    expect(arr);

    expect(!anv_arr_pop(arr, item_t));

    anv_arr_destroy(arr);
}

ANV_TESTSUITE_FIXTURE(anv_arr_pop_ok)
{
    anv_arr_t arr = anv_arr_new(10, sizeof(item_t));
    expect(arr);

    expect(anv_arr_push_new(arr, item_t, { .a = 10 }) == ANV_ARR_RESULT_OK);
    expect(anv_arr_push_new(arr, item_t, { .a = 20 }) == ANV_ARR_RESULT_OK);
    expect(anv_arr_length(arr) == 2);

    item_t *found_item_1 = anv_arr_pop(arr, item_t);
    expect(found_item_1);
    expect(found_item_1->a == 20);
    expect(anv_arr_length(arr) == 1);

    item_t *found_item_2 = anv_arr_pop(arr, item_t);
    expect(found_item_2);
    expect(found_item_2->a == 10);
    expect(anv_arr_length(arr) == 0);

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

ANV_TESTSUITE_FIXTURE(anv_arr_remove_when_1_item_only_return_ok)
{
    anv_arr_t arr = anv_arr_new(10, sizeof(item_t));
    expect(arr);

    item_t item1 = { .a = 69 };
    expect(anv_arr_push(arr, &item1) == ANV_ARR_RESULT_OK);

    expect(anv_arr_remove(arr, 0) == ANV_ARR_RESULT_OK);

    anv_arr_destroy(arr);
}

ANV_TESTSUITE_FIXTURE(anv_arr_remove_multiple_items_ok)
{
    anv_arr_t arr = anv_arr_new(10, sizeof(item_t));
    expect(arr);

    item_t item1 = { .a = 69 };
    expect(anv_arr_push(arr, &item1) == ANV_ARR_RESULT_OK);
    item_t item2 = { .a = 690 };
    expect(anv_arr_push(arr, &item2) == ANV_ARR_RESULT_OK);
    item_t item3 = { .a = 6900 };
    expect(anv_arr_push(arr, &item3) == ANV_ARR_RESULT_OK);
    expect(anv_arr_length(arr) == 3);

    expect(anv_arr_remove(arr, 1) == ANV_ARR_RESULT_OK);
    expect(anv_arr_length(arr) == 2);

    item_t *found_item = anv_arr_get(arr, item_t, 1);
    expect(found_item);
    expect(found_item->a == 6900);

    anv_arr_destroy(arr);
}

ANV_TESTSUITE_FIXTURE(anv_arr_remove_with_null_arr_returns_invalid_params)
{
    expect(anv_arr_remove(NULL, 0) == ANV_ARR_RESULT_INVALID_PARAMS);
}

ANV_TESTSUITE_FIXTURE(anv_arr_remove_with_invalid_arr_returns_invalid_params)
{
    void *mem = malloc(10);
    expect(mem);

    expect(anv_arr_remove(mem, 0) == ANV_ARR_RESULT_INVALID_PARAMS);

    free(mem);
}

ANV_TESTSUITE_FIXTURE(anv_arr_remove_when_empty_array_return_out_of_bounds_err)
{
    anv_arr_t arr = anv_arr_new(10, sizeof(item_t));
    expect(arr);

    item_t item1 = { .a = 69 };
    expect(anv_arr_push(arr, &item1) == ANV_ARR_RESULT_OK);
    item_t item2 = { .a = 690 };
    expect(anv_arr_push(arr, &item2) == ANV_ARR_RESULT_OK);

    expect(anv_arr_remove(arr, 2) == ANV_ARR_RESULT_INDEX_OUT_OF_BOUNDS);

    anv_arr_destroy(arr);
}

ANV_TESTSUITE_FIXTURE(
    anv_arr_remove_when_index_out_of_bounds_return_out_of_bounds_err
)
{
    anv_arr_t arr = anv_arr_new(10, sizeof(item_t));
    expect(arr);

    expect(anv_arr_remove(arr, 0) == ANV_ARR_RESULT_INDEX_OUT_OF_BOUNDS);

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
    ANV_TESTSUITE_REGISTER(anv_arr_insert_with_null_arr_is_param_error),
    ANV_TESTSUITE_REGISTER(anv_arr_insert_with_invalid_arr_is_param_error),
    ANV_TESTSUITE_REGISTER(anv_arr_insert_with_null_item_is_ok),
    ANV_TESTSUITE_REGISTER(anv_arr_insert_with_out_of_bounds_index_is_error),
    ANV_TESTSUITE_REGISTER(anv_arr_insert_with_empty_array_is_ok),
    ANV_TESTSUITE_REGISTER(anv_arr_insert_with_not_empty_array_is_ok),
    ANV_TESTSUITE_REGISTER(anv_arr_insert_multiple_check_ordering_ok),
    ANV_TESTSUITE_REGISTER(anv_arr_insert_with_array_expansion_ok),
    ANV_TESTSUITE_REGISTER(anv_arr_insert_new_ok),
    ANV_TESTSUITE_REGISTER(anv_arr_push_with_null_arr_is_param_error),
    ANV_TESTSUITE_REGISTER(anv_arr_push_with_null_item_is_ok),
    ANV_TESTSUITE_REGISTER(anv_arr_push_with_invalid_arr_is_param_error),
    ANV_TESTSUITE_REGISTER(anv_arr_push_with_arr_low_capacity_is_extended),
    ANV_TESTSUITE_REGISTER(anv_arr_push_new_with_new_struct_is_ok),
    ANV_TESTSUITE_REGISTER(anv_arr_pop_when_null_array_return_null),
    ANV_TESTSUITE_REGISTER(anv_arr_pop_when_invalid_array_return_null),
    ANV_TESTSUITE_REGISTER(anv_arr_pop_when_empty_array_return_null),
    ANV_TESTSUITE_REGISTER(anv_arr_pop_ok),
    ANV_TESTSUITE_REGISTER(anv_arr_config_reallocator_fn_custom_ok),
    ANV_TESTSUITE_REGISTER(anv_arr_config_reallocator_fn_restore_default_ok),
    ANV_TESTSUITE_REGISTER(anv_arr_get_with_no_elements_returns_null),
    ANV_TESTSUITE_REGISTER(anv_arr_get_with_null_arr_returns_null),
    ANV_TESTSUITE_REGISTER(anv_arr_get_with_invalid_arr_returns_null),
    ANV_TESTSUITE_REGISTER(anv_arr_get_with_element_returns_correct_element),
    ANV_TESTSUITE_REGISTER(anv_arr_for_loop_with_no_elements_is_ok),
    ANV_TESTSUITE_REGISTER(anv_arr_for_loop_with_elements_is_ok),
    ANV_TESTSUITE_REGISTER(anv_arr_remove_when_1_item_only_return_ok),
    ANV_TESTSUITE_REGISTER(anv_arr_remove_multiple_items_ok),
    ANV_TESTSUITE_REGISTER(anv_arr_remove_with_null_arr_returns_invalid_params),
    ANV_TESTSUITE_REGISTER(anv_arr_remove_with_invalid_arr_returns_invalid_params),
    ANV_TESTSUITE_REGISTER(
        anv_arr_remove_when_empty_array_return_out_of_bounds_err
    ),
    ANV_TESTSUITE_REGISTER(
        anv_arr_remove_when_index_out_of_bounds_return_out_of_bounds_err
    ),
);

int
main(void)
{
    anv_testsuite_catch_crashes();
    ANV_TESTSUITE_RUN(tests_anv_arr, stdout);
}
