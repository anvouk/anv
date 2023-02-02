#include "../include/anv_testsuite_2.h"

#include <stdlib.h>

// disable debug assertion to enable testing invalid cases in prod.
#define anv_meta__assert(cond, errmsg) ((void)(cond))

#define ANV_METALLOC_IMPLEMENTATION
#include "../include/anv_metalloc.h"

typedef struct metadata_t {
    int a, b;
} metadata_t;

ANV_TESTSUITE_FIXTURE(anv_meta_malloc_simple_ok)
{
    metadata_t meta = { 10, 20 };
    void *mem = anv_meta_malloc(&meta, sizeof(metadata_t), 100);

    expect(mem);

    anv_meta_free(mem);
}

ANV_TESTSUITE_FIXTURE(anv_meta_malloc_simple_fail_zero_memory)
{
    metadata_t meta = { 10, 20 };
    void *mem = anv_meta_malloc(&meta, sizeof(metadata_t), 0);

    expect(!mem);
}

ANV_TESTSUITE_FIXTURE(anv_meta_malloc_check_metadata_is_valid)
{
    metadata_t meta = { 10, 20 };
    void *mem = anv_meta_malloc(&meta, sizeof(metadata_t), 100);

    expect(mem);
    expect(anv_meta_isvalid(mem));

    anv_meta_free(mem);
}

ANV_TESTSUITE_FIXTURE(anv_meta_malloc_check_metadata_sz)
{
    metadata_t meta = { 10, 20 };
    void *mem = anv_meta_malloc(&meta, sizeof(metadata_t), 100);

    expect(mem);
    expect(anv_meta_getsz(mem) == sizeof(metadata_t));

    anv_meta_free(mem);
}

ANV_TESTSUITE_FIXTURE(anv_meta_malloc_check_metadata_data)
{
    metadata_t meta = { 10, 20 };
    void *mem = anv_meta_malloc(&meta, sizeof(metadata_t), 100);

    expect(mem);
    metadata_t *retrieved_metadata = (metadata_t *)anv_meta_get(mem);
    expect(retrieved_metadata);
    expect(retrieved_metadata->a == 10);
    expect(retrieved_metadata->b == 20);

    anv_meta_free(mem);
}

ANV_TESTSUITE_FIXTURE(anv_meta_malloc_empty_metadata_struct_is_zeroed)
{
    void *mem = anv_meta_malloc(NULL, sizeof(metadata_t), 100);

    expect(mem);
    // passing NULL as metadata is basically saying you want a zeroed struct
    // in out case.
    metadata_t *retrieved_metadata = (metadata_t *)anv_meta_get(mem);
    expect(retrieved_metadata);
    expect(retrieved_metadata->a == 0);
    expect(retrieved_metadata->b == 0);

    anv_meta_free(mem);
}

ANV_TESTSUITE_FIXTURE(anv_meta_malloc_empty_metadata_set_new)
{
    void *mem = anv_meta_malloc(NULL, sizeof(metadata_t), 100);

    expect(mem);
    // passing NULL as metadata is basically saying you want a zeroed struct
    // in out case.
    metadata_t *retrieved_metadata = (metadata_t *)anv_meta_get(mem);
    expect(retrieved_metadata);
    expect(retrieved_metadata->a == 0);
    expect(retrieved_metadata->b == 0);

    metadata_t meta = { 10, 20 };
    anv_meta_set(mem, &meta);
    expect(mem);
    metadata_t *retrieved_metadata_new = (metadata_t *)anv_meta_get(mem);
    expect(retrieved_metadata_new);
    expect(retrieved_metadata_new->a == 10);
    expect(retrieved_metadata_new->b == 20);

    anv_meta_free(mem);
}

ANV_TESTSUITE_FIXTURE(anv_meta_malloc_no_metadata_fail)
{
    void *mem = anv_meta_malloc(NULL, 0, 100);

    expect(!mem);
}

ANV_TESTSUITE_FIXTURE(anv_meta_malloc_change_data)
{
    metadata_t meta = { 10, 20 };
    void *mem = anv_meta_malloc(&meta, sizeof(metadata_t), 100);
    expect(mem);

    metadata_t *retrieved_metadata = (metadata_t *)anv_meta_get(mem);
    expect(retrieved_metadata);
    expect(retrieved_metadata->a == 10);
    expect(retrieved_metadata->b == 20);

    memset(mem, 69, 100);
    int correct_sum = 69 * 100;
    int found_sum = 0;
    int c = 0;
    for (char *s = mem; c < 100; ++s, ++c) {
        found_sum += *s;
    }
    expect(found_sum == correct_sum);

    anv_meta_free(mem);
}

ANV_TESTSUITE_FIXTURE(anv_meta_free_null_ok)
{
    anv_meta_free(NULL);
    expect(1);
}

ANV_TESTSUITE_FIXTURE(anv_meta_isvalid_ok)
{
    metadata_t meta = { 10, 20 };
    void *mem = anv_meta_malloc(&meta, sizeof(metadata_t), 100);
    expect(mem);

    expect(anv_meta_isvalid(mem));

    anv_meta_free(mem);
}

ANV_TESTSUITE_FIXTURE(anv_meta_isvalid_ko_when_null_mem)
{
    expect(!anv_meta_isvalid(NULL));
}

ANV_TESTSUITE_FIXTURE(anv_meta_isvalid_ko_when_invalid_mem)
{
    void *mem = malloc(100);
    expect(mem);
    expect(!anv_meta_isvalid(mem));
    free(mem);
}

ANV_TESTSUITE_FIXTURE(anv_meta_getsz_ok)
{
    metadata_t meta = { 10, 20 };
    void *mem = anv_meta_malloc(&meta, sizeof(metadata_t), 100);
    expect(mem);

    expect(anv_meta_getsz(mem) == sizeof(metadata_t));

    anv_meta_free(mem);
}

ANV_TESTSUITE_FIXTURE(anv_meta_getsz_when_null_mem_return_0)
{
    expect(anv_meta_getsz(NULL) == 0);
}

ANV_TESTSUITE_FIXTURE(anv_meta_getsz_when_invalid_mem_return_0)
{
    void *mem = malloc(10);
    expect(mem);
    expect(anv_meta_getsz(mem) == 0);
    free(mem);
}

ANV_TESTSUITE_FIXTURE(anv_meta_get_ok_when_metadata_exists)
{
    metadata_t meta = { 10, 20 };
    void *mem = anv_meta_malloc(&meta, sizeof(metadata_t), 100);
    expect(mem);

    metadata_t *found_meta = (metadata_t *)anv_meta_get(mem);
    expect(found_meta);
    expect(found_meta->a == 10);
    expect(found_meta->b == 20);

    anv_meta_free(mem);
}

ANV_TESTSUITE_FIXTURE(anv_meta_get_when_no_metadata_return_empty_metadata)
{
    void *mem = anv_meta_malloc(NULL, sizeof(metadata_t), 100);
    expect(mem);

    metadata_t *found_meta = (metadata_t *)anv_meta_get(mem);
    expect(found_meta);
    expect(found_meta->a == 0);
    expect(found_meta->b == 0);

    anv_meta_free(mem);
}

ANV_TESTSUITE_FIXTURE(anv_meta_get_when_null_mem_return_0)
{
    expect(anv_meta_get(NULL) == 0);
}

ANV_TESTSUITE_FIXTURE(anv_meta_get_when_invalid_mem_return_0)
{
    void *mem = malloc(10);
    expect(mem);
    expect(anv_meta_get(mem) == 0);
    free(mem);
}

ANV_TESTSUITE_FIXTURE(anv_meta_set_ok)
{
    metadata_t meta = { 10, 20 };
    void *mem = anv_meta_malloc(&meta, sizeof(metadata_t), 100);
    expect(mem);

    meta.a = 100;
    expect(anv_meta_set(mem, &meta) == ANV_META_RESULT_OK);
    metadata_t *found_meta = (metadata_t *)anv_meta_get(mem);
    expect(found_meta);
    expect(found_meta->a == 100);
    expect(found_meta->b == 20);

    anv_meta_free(mem);
}

ANV_TESTSUITE_FIXTURE(anv_meta_set_ok_when_null_metadata)
{
    metadata_t meta = { 10, 20 };
    void *mem = anv_meta_malloc(&meta, sizeof(metadata_t), 100);
    expect(mem);

    expect(anv_meta_set(mem, NULL) == ANV_META_RESULT_OK);
    metadata_t *found_meta = (metadata_t *)anv_meta_get(mem);
    expect(found_meta);
    expect(found_meta->a == 0);
    expect(found_meta->b == 0);

    anv_meta_free(mem);
}

ANV_TESTSUITE_FIXTURE(anv_meta_set_when_null_mem_return_invalid_params_err)
{
    metadata_t meta = { 10, 20 };
    expect(anv_meta_set(NULL, &meta) == ANV_META_RESULT_INVALID_PARAMS);
}

ANV_TESTSUITE_FIXTURE(anv_meta_set_when_invalid_mem_return_invalid_params_err)
{
    metadata_t meta = { 10, 20 };

    void *mem = malloc(10);
    expect(mem);
    expect(anv_meta_set(mem, &meta) == ANV_META_RESULT_INVALID_PARAMS);
    free(mem);
}

ANV_TESTSUITE_FIXTURE(anv_meta_get_offset_ok)
{
    metadata_t meta = { 10, 20 };
    void *mem = anv_meta_malloc(&meta, sizeof(metadata_t), 100);

    expect(mem);
    metadata_t *retrieved_metadata = (metadata_t *)anv_meta_get(mem);
    expect(retrieved_metadata);
    expect(retrieved_metadata->a == 10);
    expect(retrieved_metadata->b == 20);

    size_t offset = anv_meta_get_offset(mem);
    metadata_t *meta_manual = (metadata_t *)((size_t)mem - offset);
    metadata_t *meta_method = (metadata_t *)anv_meta_get(mem);
    expect(meta_manual == meta_method);

    anv_meta_free(mem);
}

ANV_TESTSUITE_FIXTURE(anv_meta_get_offset_when_null_mem_return_0)
{
    expect(anv_meta_get_offset(NULL) == 0);
}

ANV_TESTSUITE_FIXTURE(anv_meta_get_offset_when_invalid_mem_return_0)
{
    void *mem = malloc(10);
    expect(mem);
    expect(anv_meta_get_offset(mem) == 0);
    free(mem);
}

ANV_TESTSUITE_FIXTURE(anv_meta_realloc_simple_ok)
{
    metadata_t meta = { 10, 20 };
    void *mem = anv_meta_malloc(&meta, sizeof(metadata_t), 100);

    expect(mem);
    metadata_t *retrieved_metadata = (metadata_t *)anv_meta_get(mem);
    expect(retrieved_metadata);
    expect(retrieved_metadata->a == 10);
    expect(retrieved_metadata->b == 20);

    void *new_mem = anv_meta_realloc(mem, 200);
    expect(new_mem);

    anv_meta_free(new_mem);
}

ANV_TESTSUITE_FIXTURE(anv_meta_realloc_simple_fail_on_null_mem)
{
    void *new_mem = anv_meta_realloc(NULL, 200);
    expect(!new_mem);
}

ANV_TESTSUITE_FIXTURE(anv_meta_realloc_get_offset_ok)
{
    metadata_t meta = { 10, 20 };
    void *mem = anv_meta_malloc(&meta, sizeof(metadata_t), 100);

    expect(mem);
    metadata_t *retrieved_metadata = (metadata_t *)anv_meta_get(mem);
    expect(retrieved_metadata);
    expect(retrieved_metadata->a == 10);
    expect(retrieved_metadata->b == 20);

    void *new_mem = anv_meta_realloc(mem, 200);
    expect(anv_meta_isvalid(new_mem));

    size_t offset = anv_meta_get_offset(new_mem);
    metadata_t *meta_manual = (metadata_t *)((size_t)new_mem - offset);
    metadata_t *meta_method = (metadata_t *)anv_meta_get(new_mem);
    expect(meta_manual == meta_method);

    anv_meta_free(new_mem);
}

ANV_TESTSUITE_FIXTURE(anv_meta_realloc_check_metadata_is_valid)
{
    metadata_t meta = { 10, 20 };
    void *mem = anv_meta_malloc(&meta, sizeof(metadata_t), 100);

    expect(mem);
    metadata_t *retrieved_metadata = (metadata_t *)anv_meta_get(mem);
    expect(retrieved_metadata);
    expect(retrieved_metadata->a == 10);
    expect(retrieved_metadata->b == 20);

    void *new_mem = anv_meta_realloc(mem, 200);
    expect(anv_meta_isvalid(new_mem));

    anv_meta_free(new_mem);
}

ANV_TESTSUITE_FIXTURE(anv_meta_realloc_check_metadata_sz)
{
    metadata_t meta = { 10, 20 };
    void *mem = anv_meta_malloc(&meta, sizeof(metadata_t), 100);

    expect(mem);
    metadata_t *retrieved_metadata = (metadata_t *)anv_meta_get(mem);
    expect(retrieved_metadata);
    expect(retrieved_metadata->a == 10);
    expect(retrieved_metadata->b == 20);

    void *new_mem = anv_meta_realloc(mem, 200);
    expect(new_mem);
    expect(anv_meta_getsz(new_mem) == sizeof(metadata_t));

    anv_meta_free(new_mem);
}

ANV_TESTSUITE_FIXTURE(anv_meta_realloc_check_metadata_data)
{
    metadata_t meta = { 10, 20 };
    void *mem = anv_meta_malloc(&meta, sizeof(metadata_t), 100);

    expect(mem);
    metadata_t *retrieved_metadata = (metadata_t *)anv_meta_get(mem);
    expect(retrieved_metadata);
    expect(retrieved_metadata->a == 10);
    expect(retrieved_metadata->b == 20);

    void *new_mem = anv_meta_realloc(mem, 200);
    expect(new_mem);
    metadata_t *retrieved_metadata_new = (metadata_t *)anv_meta_get(new_mem);
    expect(retrieved_metadata_new);
    expect(retrieved_metadata_new->a == 10);
    expect(retrieved_metadata_new->b == 20);

    anv_meta_free(new_mem);
}

ANV_TESTSUITE_FIXTURE(anv_meta_realloc_change_metadata)
{
    metadata_t meta = { 10, 20 };
    void *mem = anv_meta_malloc(&meta, sizeof(metadata_t), 100);

    expect(mem);
    metadata_t *retrieved_metadata = (metadata_t *)anv_meta_get(mem);
    expect(retrieved_metadata);
    expect(retrieved_metadata->a == 10);
    expect(retrieved_metadata->b == 20);

    void *new_mem = anv_meta_realloc(mem, 200);
    expect(new_mem);
    metadata_t *retrieved_metadata_new = (metadata_t *)anv_meta_get(new_mem);
    expect(retrieved_metadata_new);
    expect(retrieved_metadata_new->a == 10);
    expect(retrieved_metadata_new->b == 20);

    metadata_t meta2 = { 100, 200 };
    expect(anv_meta_set(new_mem, &meta2) == ANV_META_RESULT_OK);
    metadata_t *retrieved_metadata_new2 = (metadata_t *)anv_meta_get(new_mem);
    expect(retrieved_metadata_new2);
    expect(retrieved_metadata_new2->a == 100);
    expect(retrieved_metadata_new2->b == 200);

    anv_meta_free(new_mem);
}

ANV_TESTSUITE_FIXTURE(anv_meta_realloc_set_metadata_when_before_was_not_present)
{
    void *mem = anv_meta_malloc(NULL, sizeof(metadata_t), 100);

    expect(mem);
    metadata_t *retrieved_metadata = (metadata_t *)anv_meta_get(mem);
    expect(retrieved_metadata);
    expect(retrieved_metadata->a == 0);
    expect(retrieved_metadata->b == 0);

    void *new_mem = anv_meta_realloc(mem, 200);
    expect(new_mem);
    metadata_t *retrieved_metadata_new = (metadata_t *)anv_meta_get(new_mem);
    expect(retrieved_metadata_new);
    expect(retrieved_metadata_new->a == 0);
    expect(retrieved_metadata_new->b == 0);

    metadata_t meta2 = { 100, 200 };
    expect(anv_meta_set(new_mem, &meta2) == ANV_META_RESULT_OK);
    metadata_t *retrieved_metadata_new2 = (metadata_t *)anv_meta_get(new_mem);
    expect(retrieved_metadata_new2);
    expect(retrieved_metadata_new2->a == 100);
    expect(retrieved_metadata_new2->b == 200);

    anv_meta_free(new_mem);
}

ANV_TESTSUITE_FIXTURE(anv_meta_realloc_change_data)
{
    metadata_t meta = { 10, 20 };
    void *mem = anv_meta_malloc(&meta, sizeof(metadata_t), 10);

    expect(mem);
    metadata_t *retrieved_metadata = (metadata_t *)anv_meta_get(mem);
    expect(retrieved_metadata);
    expect(retrieved_metadata->a == 10);
    expect(retrieved_metadata->b == 20);

    void *new_mem = anv_meta_realloc(mem, 20);
    expect(new_mem);
    metadata_t *retrieved_metadata_new = (metadata_t *)anv_meta_get(new_mem);
    expect(retrieved_metadata_new);
    expect(retrieved_metadata_new->a == 10);
    expect(retrieved_metadata_new->b == 20);

    memset(new_mem, 69, 20);
    int correct_sum = 69 * 20;
    int found_sum = 0;
    int c = 0;
    for (char *s = new_mem; c < 20; ++s, ++c) {
        found_sum += *s;
    }
    expect(found_sum == correct_sum);

    anv_meta_free(new_mem);
}

ANV_TESTSUITE(
    tests_anv_metalloc,
    ANV_TESTSUITE_REGISTER(anv_meta_malloc_simple_ok),
    ANV_TESTSUITE_REGISTER(anv_meta_malloc_simple_fail_zero_memory),
    ANV_TESTSUITE_REGISTER(anv_meta_malloc_check_metadata_is_valid),
    ANV_TESTSUITE_REGISTER(anv_meta_malloc_check_metadata_sz),
    ANV_TESTSUITE_REGISTER(anv_meta_malloc_check_metadata_data),
    ANV_TESTSUITE_REGISTER(anv_meta_malloc_empty_metadata_struct_is_zeroed),
    ANV_TESTSUITE_REGISTER(anv_meta_malloc_empty_metadata_set_new),
    ANV_TESTSUITE_REGISTER(anv_meta_malloc_no_metadata_fail),
    ANV_TESTSUITE_REGISTER(anv_meta_malloc_change_data),
    ANV_TESTSUITE_REGISTER(anv_meta_free_null_ok),
    ANV_TESTSUITE_REGISTER(anv_meta_isvalid_ok),
    ANV_TESTSUITE_REGISTER(anv_meta_isvalid_ko_when_null_mem),
    ANV_TESTSUITE_REGISTER(anv_meta_isvalid_ko_when_invalid_mem),
    ANV_TESTSUITE_REGISTER(anv_meta_getsz_ok),
    ANV_TESTSUITE_REGISTER(anv_meta_getsz_when_null_mem_return_0),
    ANV_TESTSUITE_REGISTER(anv_meta_getsz_when_invalid_mem_return_0),
    ANV_TESTSUITE_REGISTER(anv_meta_get_ok_when_metadata_exists),
    ANV_TESTSUITE_REGISTER(anv_meta_get_when_no_metadata_return_empty_metadata),
    ANV_TESTSUITE_REGISTER(anv_meta_get_when_null_mem_return_0),
    ANV_TESTSUITE_REGISTER(anv_meta_get_when_invalid_mem_return_0),
    ANV_TESTSUITE_REGISTER(anv_meta_set_ok),
    ANV_TESTSUITE_REGISTER(anv_meta_set_ok_when_null_metadata),
    ANV_TESTSUITE_REGISTER(anv_meta_set_when_null_mem_return_invalid_params_err
    ),
    ANV_TESTSUITE_REGISTER(
        anv_meta_set_when_invalid_mem_return_invalid_params_err
    ),
    ANV_TESTSUITE_REGISTER(anv_meta_get_offset_ok),
    ANV_TESTSUITE_REGISTER(anv_meta_get_offset_when_null_mem_return_0),
    ANV_TESTSUITE_REGISTER(anv_meta_get_offset_when_invalid_mem_return_0),
    ANV_TESTSUITE_REGISTER(anv_meta_realloc_simple_ok),
    ANV_TESTSUITE_REGISTER(anv_meta_realloc_simple_fail_on_null_mem),
    ANV_TESTSUITE_REGISTER(anv_meta_realloc_get_offset_ok),
    ANV_TESTSUITE_REGISTER(anv_meta_realloc_check_metadata_is_valid),
    ANV_TESTSUITE_REGISTER(anv_meta_realloc_check_metadata_sz),
    ANV_TESTSUITE_REGISTER(anv_meta_realloc_check_metadata_data),
    ANV_TESTSUITE_REGISTER(anv_meta_realloc_change_metadata),
    ANV_TESTSUITE_REGISTER(
        anv_meta_realloc_set_metadata_when_before_was_not_present
    ),
    ANV_TESTSUITE_REGISTER(anv_meta_realloc_change_data),
);

int
main(void)
{
    anv_testsuite_catch_crashes();
    ANV_TESTSUITE_RUN(tests_anv_metalloc, stdout);
}
