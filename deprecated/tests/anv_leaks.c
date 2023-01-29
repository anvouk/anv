#include "../include/anv_testsuite_2.h"

#define ANV_LEAKS_IMPLEMENTATION
#include "../include/anv_leaks.h"

static FILE *anv_leaks_out = NULL;

ANV_TESTSUITE_FIXTURE(anv_leaks_test_init)
{
    expect_msg(!anv_leaks_out, "anv_leaks already initialized");

    anv_leaks_out = fopen("leaks.log", "w");
    expect_msg(anv_leaks_out, "failed creating temp file");

    anv_leaks_init(anv_leaks_out);
}

ANV_TESTSUITE_FIXTURE(anv_leaks_test_shutdown)
{
    expect_msg(anv_leaks_out, "anv_leaks not initialized");

    /* before shutting down let's have a quick peek... */
    anv_leaks_quickpeek();

    fclose(anv_leaks_out);
    anv_leaks_out = NULL;
}

ANV_TESTSUITE_FIXTURE(anv_leaks_test_malloc)
{
    anv_leaks_stats stats;
    void *mem = NULL;

    mem = malloc(100);
    expect(mem);

    anv_leaks_get_stats(&stats);
    expect(stats.total_allocated == 100);
    expect(stats.total_freed == 0);
    expect(stats.malloc_count == 1);
    expect(stats.free_count == 0);
    expect(stats.calloc_count == 0);
    expect(stats.realloc_count == 0);

    free(mem);

    anv_leaks_get_stats(&stats);
    expect(stats.total_allocated == 100);
    expect(stats.total_freed == 100);
    expect(stats.malloc_count == 1);
    expect(stats.free_count == 1);
    expect(stats.calloc_count == 0);
    expect(stats.realloc_count == 0);
}

ANV_TESTSUITE_FIXTURE(anv_leaks_test_realloc)
{
    anv_leaks_stats stats;
    void *mem = NULL;

    mem = malloc(50);
    expect(mem);

    mem = realloc(mem, 200);
    expect(mem);

    anv_leaks_get_stats(&stats);
    expect(stats.total_allocated == 300);
    expect(stats.total_freed == 100);
    expect(stats.realloc_count == 1);

    free(mem);

    anv_leaks_get_stats(&stats);
    expect(stats.total_allocated == 300);
    expect(stats.total_freed == 300);
    expect(stats.free_count == 2);
    expect(stats.realloc_count == 1);
}

ANV_TESTSUITE_FIXTURE(anv_leaks_test_calloc)
{
    anv_leaks_stats stats;
    void *mem = NULL;

    mem = calloc(2, 10);
    expect(mem);

    anv_leaks_get_stats(&stats);
    expect(stats.total_allocated == 320);
    expect(stats.total_freed == 300);
    expect(stats.free_count == 2);
    expect(stats.calloc_count == 1);

    free(mem);

    anv_leaks_get_stats(&stats);
    expect(stats.total_allocated == 320);
    expect(stats.total_freed == 320);
    expect(stats.free_count == 3);
    expect(stats.calloc_count == 1);
}

static void
anv_leaks_test__print_leaks(anv_leak_info **leaks, size_t leaks_count)
{
    for (size_t i = 0; i < leaks_count; ++i) {
        anv_leak_info *l = leaks[i];
        fprintf(
            anv_leaks_out,
            "[%s:%d] [%p] = %d\n",
            l->filename,
            l->line,
            l->address,
            (int)l->bytes
        );
    }
}

ANV_TESTSUITE_FIXTURE(anv_leaks_test_get_leaks)
{
    anv_leak_info **leaks;
    size_t leaks_count;

    anv_leaks_get_leaks(&leaks, &leaks_count);

    /* this should not trigger */
    anv_leaks_test__print_leaks(leaks, leaks_count);
    expect_msg(leaks_count == 0, "we 'have' memory leaks while we should not!");

    void *mem = malloc(10);
    expect(mem);

    /* now we have a 'memory leak' */
    anv_leaks_free_info(leaks, leaks_count);
    anv_leaks_get_leaks(&leaks, &leaks_count);
    expect(leaks_count == 1);
    anv_leaks_test__print_leaks(leaks, leaks_count);
    anv_leaks_quickpeek();

    mem = realloc(mem, 20);
    expect(mem);

    /* still 1 leak */
    anv_leaks_free_info(leaks, leaks_count);
    anv_leaks_get_leaks(&leaks, &leaks_count);
    expect(leaks_count == 1);
    /* SHOULD trigger */
    anv_leaks_test__print_leaks(leaks, leaks_count);

    /* not anymore */
    free(mem);
    anv_leaks_free_info(leaks, leaks_count);
    anv_leaks_get_leaks(&leaks, &leaks_count);
    expect(leaks_count == 0);

    /* should't trigger */
    anv_leaks_test__print_leaks(leaks, leaks_count);

    anv_leaks_free_info(leaks, leaks_count);
}

ANV_TESTSUITE_FIXTURE(anv_leaks_test_multi_leaks)
{
    anv_leak_info **leaks;
    size_t leaks_count;
    void *mem1, *mem2, *mem3;

    mem1 = malloc(11);
    mem2 = malloc(22);
    mem2 = realloc(mem2, 23);

    mem3 = calloc(4, 11);

    anv_leaks_get_leaks(&leaks, &leaks_count);
    expect(leaks_count == 3);
    fprintf(anv_leaks_out, "=== begin anv_leaks_test_multi_leaks ===\n");
    anv_leaks_test__print_leaks(leaks, leaks_count);
    fprintf(anv_leaks_out, "=== end anv_leaks_test_multi_leaks ===\n");

    free(mem2);
    free(mem1);
    free(mem3);

    anv_leaks_get_leaks(&leaks, &leaks_count);
    expect(leaks_count == 0);
    fprintf(anv_leaks_out, "=== begin anv_leaks_test_multi_leaks ===\n");
    anv_leaks_test__print_leaks(leaks, leaks_count);
    fprintf(anv_leaks_out, "=== end anv_leaks_test_multi_leaks ===\n");
}

ANV_TESTSUITE_FIXTURE(anv_leaks_test_check_no_leaks)
{
    anv_leaks_stats stats;
    anv_leaks_get_stats(&stats);

    expect(stats.total_allocated == stats.total_freed);
    expect(stats.malloc_count + stats.calloc_count == stats.free_count);

    anv_leak_info **leaks;
    size_t leaks_count;

    anv_leaks_get_leaks(&leaks, &leaks_count);
    expect(leaks_count == 0);
    /* should not trigger */
    anv_leaks_test__print_leaks(leaks, leaks_count);

    anv_leaks_free_info(leaks, leaks_count);
}

/* undef just in case... */
#undef malloc
#undef realloc
#undef calloc
#undef free

/* don't change invocation order carelessly! */
ANV_TESTSUITE(
    tests_anv_leaks,
    ANV_TESTSUITE_REGISTER(anv_leaks_test_init),
    /* allocation/free */
    ANV_TESTSUITE_REGISTER(anv_leaks_test_malloc),
    ANV_TESTSUITE_REGISTER(anv_leaks_test_realloc),
    ANV_TESTSUITE_REGISTER(anv_leaks_test_calloc),
    /* other */
    ANV_TESTSUITE_REGISTER(anv_leaks_test_get_leaks),
    ANV_TESTSUITE_REGISTER(anv_leaks_test_multi_leaks),
    /* shutdown checks */
    ANV_TESTSUITE_REGISTER(anv_leaks_test_check_no_leaks),
    ANV_TESTSUITE_REGISTER(anv_leaks_test_shutdown),
);

int
main(void)
{
    ANV_TESTSUITE_RUN(tests_anv_leaks, stdout);
}
