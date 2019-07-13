#include "anv_testsuite.h"

#include <stdlib.h>

#include "anv_metalloc.h"

typedef struct metadata_t {
    int a, b;
} metadata_t;

ANV_TESTSUITE_FIXTURE(anv_metalloc_test_malloc)
{
    /* alloc test */
    metadata_t meta = { 10, 20 };
	void *mem = anv_meta_malloc(&meta, sizeof(metadata_t), 100);
    expect(mem);

    expect(anv_meta_isvalid(mem));
    expect(anv_meta_getsz(mem) == sizeof(metadata_t));

    /* meta retrival test */
    meta = *(metadata_t *)anv_meta_get(mem);
    expect(meta.a == 10 && meta.b == 20);

    /* cleanup test */
    anv_meta_free(mem);
}

ANV_TESTSUITE_FIXTURE(anv_metalloc_test_realloc)
{
    /* alloc test */
    metadata_t meta = { 10, 20 };
    int *mem = anv_meta_malloc(&meta, sizeof(metadata_t), 10 * sizeof(int));
    expect(mem);

    int tot = 0;

    /* memory access test */
    for (int i = 0; i < 10; ++i) {
        mem[i] = 10 - i;
        tot += mem[i];
    }
    expect(tot == 55);

    mem = anv_meta_realloc(mem, 20);
    expect(mem);

    expect(anv_meta_getsz(mem) == sizeof(metadata_t));

    /* memory access test */
    for (int i = 10; i < 20; ++i) {
        mem[i] = 20 - i;
        tot += mem[i];
    }
    expect(tot == 110);

    /* meta retrival test */
    meta = *(metadata_t *)anv_meta_get(mem);
    expect(meta.a == 10 && meta.b == 20);

    /* cleanup test */
    anv_meta_free(mem);
}

ANV_TESTSUITE_FIXTURE(anv_metalloc_test_calloc)
{
    /* alloc test */
    metadata_t meta = { 10, 20 };
    int *mem = anv_meta_calloc(&meta, sizeof(metadata_t), 10, sizeof(int));
    expect(mem);

    /* meta retrival test */
    meta = *(metadata_t *)anv_meta_get(mem);
    expect(meta.a == 10 && meta.b == 20);

    /* memory access test */
    int tot = 0;
    for (int i = 0; i < 10; ++i) {
        mem[i] = 10 - i;
        tot += mem[i];
    }
    expect(tot == 55);

    /* cleanup test */
    anv_meta_free(mem);
}

ANV_TESTSUITE_FIXTURE(anv_metalloc_test_meta_edit)
{
    metadata_t meta = { 10, 20 };
    void *mem = anv_meta_malloc(&meta, sizeof(metadata_t), 100);
    expect(mem);

    meta = *(metadata_t *)anv_meta_get(mem);
    expect(meta.a == 10 && meta.b == 20);
    {
        metadata_t meta2 = { 100, 200 };
        anv_meta_set(mem, &meta2);
    }
    meta = *(metadata_t *)anv_meta_get(mem);
    expect(meta.a == 100 && meta.b == 200);

    anv_meta_free(mem);
}

ANV_TESTSUITE_FIXTURE(anv_metalloc_test_meta_free)
{
    void *mem = malloc(100);

    /* should behave as a normal free */
    anv_meta_free(mem);
}

ANV_TESTSUITE_FIXTURE(anv_metalloc_test_meta_malloc_no_meta)
{
    /* quite useless tbh */
    void *mem = anv_meta_malloc(NULL, 0, 100);
    expect(mem);
    anv_meta_free(mem);

    /* no metadata at allocation = memset(0) */
    mem = anv_meta_malloc(NULL, sizeof(metadata_t), 100);
    expect(mem);
    metadata_t meta_ret = *(metadata_t *)anv_meta_get(mem);
    expect(meta_ret.a == 0 && meta_ret.b == 0); /* because of memset */
    {
        /* add metadata */
        metadata_t meta = { 10, 20 };
        anv_meta_set(mem, &meta);
    }
    /* retrive metadata */
    meta_ret = *(metadata_t *)anv_meta_get(mem);
    expect(meta_ret.a == 10 && meta_ret.b == 20);

    anv_meta_free(mem);
}

ANV_TESTSUITE(tests_anv_metalloc,
    /* allocation/access/free */
    ANV_TESTSUITE_ADD(anv_metalloc_test_malloc),
    ANV_TESTSUITE_ADD(anv_metalloc_test_realloc),
    ANV_TESTSUITE_ADD(anv_metalloc_test_calloc),
    /* meta */
    ANV_TESTSUITE_ADD(anv_metalloc_test_meta_edit),
    ANV_TESTSUITE_ADD(anv_metalloc_test_meta_free),
    ANV_TESTSUITE_ADD(anv_metalloc_test_meta_malloc_no_meta),
    );
