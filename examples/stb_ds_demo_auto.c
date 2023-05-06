#include <assert.h>
#include <stdint.h>
#include <stdio.h>

// stb_alloc.h is needed before stb_ds.h implementation
#define STB_ALLOC_IMPLEMENTATION
#define STB_ALLOC_DEBUG_MODE // to enable alloc/free debug counters
#include "../repackages/stb_alloc.h"

#define STB_DS_IMPLEMENTATION
#include "../repackages/stb_ds.h"

// demo struct, can be anything or just void* with size tbh.
typedef struct item_t {
    int a;
} item_t;

// demo struct for hashmaps
typedef struct map_t {
    size_t key;
    size_t value;
} map_t;

// dumps debug allocation/free counts.
static void
dump_stb_alloc(void)
{
    // debug counters variables are not defined if not enabled.
#ifdef STB_ALLOC_DEBUG_MODE
    printf("------ stb alloc stats ------\n");
    printf(
        "alloc/freed count total: %zu/%zu\n",
        stb_alloc_count_free,
        stb_alloc_count_alloc
    );
    printf("-----------------------------\n");
#endif
}

int
main(void)
{
    // simple example
    {
        // this is really an alias to an stb_malloc call with a NULL parent ptr.
        // we can also use a struct of any type and size and store stuff in it.
        void *root = stb_malloc_global(123);
        assert(root);

        // Note: init to NULL is necessary.
        item_t *my_array = NULL;
        // magic happens here: we bind the lifetime of the array to root's
        // lifetime. when 'root' gets freed, 'my_array' will too.
        stbds_arr_new_halloc(my_array, root);
        assert(my_array);

        // add enough elements to ensure memory reallocation
        for (size_t i = 0; i < 200; ++i) {
            arrpush(my_array, (item_t) { (int)i * 2 });
        }

        // do some operations on arrays
        arrdel(my_array, 20);
        arrdel(my_array, 120);
        printf("array sz: %zu/%zu\n", arrlen(my_array), arrcap(my_array));

        // free root pointer and the array as a consequence.
        stb_free(root);
        // dump debug allocations counters
        dump_stb_alloc();
        assert(stb_alloc_count_alloc == stb_alloc_count_free);
    }

    // simple example, array is also root
    {
        item_t *my_array = NULL;
        // passing NULL as parent context is the same a using stb_malloc_global
        stbds_arr_new_halloc(my_array, NULL);
        assert(my_array);

        for (size_t i = 0; i < 200; ++i) {
            arrpush(my_array, (item_t) { (int)i * 2 });
        }

        // do some operations on arrays
        arrdel(my_array, 20);
        arrdel(my_array, 120);
        printf("array sz: %zu/%zu\n", arrlen(my_array), arrcap(my_array));

        // Important: do not forget to use arrhalloc() to retrieve allocs info
        // from array.
        stb_free(stbds_header(my_array));
        dump_stb_alloc();
        assert(stb_alloc_count_alloc == stb_alloc_count_free);
    }

    // simple array of allocated elements
    {
        void *root = stb_malloc_global(123);
        assert(root);

        item_t **my_array = NULL;
        stbds_arr_new_halloc(my_array, root);
        assert(my_array);

        for (size_t i = 0; i < 300; ++i) {
            // we bind a new memory block to the array's lifetime.
            // a leaf memory block is an allocated object which cannot have
            // other children attached to. It is an optimization since we
            // know the array item's won't have further children's attached to.
            // stbds_header() is necessary to retrieve the stb alloc headers
            // from stb arrays to use for regular stb alloc functions.
            item_t *item
                = stb_malloc_leaf(stbds_header(my_array), sizeof(item_t));
            assert(item);
            arrpush(my_array, item);
        }

        // do some operations on arrays.
        // Note: removing an item from the array does not delete it
        // automatically, this because the item's lifetime is tied to the
        // array's.
        arrdel(my_array, 1);
        arrdel(my_array, 20);
        printf("array sz: %zu/%zu\n", arrlen(my_array), arrcap(my_array));

        // frees up automatically:
        // - all items in array
        // - the array
        // - the root object
        stb_free(root);
        dump_stb_alloc();
        assert(stb_alloc_count_alloc == stb_alloc_count_free);
    }

    // crazier stuff with arrays
    {
        // we're now creating an array of array of allocated structs. By freeing
        // up the root object, will automatically free everything else.
        size_t outer_sz = 100;
        size_t inner_sz = 50;

        void *root = stb_malloc_global(1);
        assert(root);

        item_t ***outer_arr = NULL;
        stbds_arr_new_halloc(outer_arr, root);
        assert(outer_arr);

        for (size_t i = 0; i < outer_sz; ++i) {
            item_t **inner_arr = NULL;
            stbds_arr_new_halloc(inner_arr, stbds_header(outer_arr));
            assert(inner_arr);
            arrput(outer_arr, inner_arr);

            for (size_t j = 0; j < inner_sz; ++j) {
                // nofree is another variant (used as example here). See
                // stb alloc docs for more info.
                item_t *item = stb_malloc_nofree(
                    stbds_header(inner_arr), sizeof(item_t)
                );
                assert(item);
                item->a = j;
                arrpush(inner_arr, item);
            }
        }

        // Note: removing an item from the array does not delete it
        // automatically
        printf(
            "[before deletes] array sz: %zu/%zu\n",
            arrlen(outer_arr),
            arrcap(outer_arr)
        );
        dump_stb_alloc();
        arrdel(outer_arr, 10);
        printf(
            "[after 1st delete] array sz: %zu/%zu\n",
            arrlen(outer_arr),
            arrcap(outer_arr)
        );
        dump_stb_alloc();
        arrdel(outer_arr, 20);
        printf(
            "[after 2nd delete] array sz: %zu/%zu\n",
            arrlen(outer_arr),
            arrcap(outer_arr)
        );
        dump_stb_alloc();

        // works still
        stb_free(root);
        dump_stb_alloc();
        assert(stb_alloc_count_alloc == stb_alloc_count_free);
    }

    // simple hashmap example
    {
        void *root = stb_malloc_global(1);
        assert(root);

        // Note: init to NULL is, again, necessary.
        map_t *my_map = NULL;
        // in the same fashion as stbds_arr_new_halloc() for arrays, we prepare
        // the map to use hallocs.
        sh_new_halloc(my_map, root);
        assert(my_map);

        // insert stuff
        for (size_t i = 0; i < 200; ++i) {
            hmput(my_map, i, i * 2);
        }

        // retrieve stuff
        for (size_t i = 0; i < 200; ++i) {
            size_t val = hmget(my_map, i);
            assert(val == i * 2);
        }

        stb_free(root);
        dump_stb_alloc();
        assert(stb_alloc_count_alloc == stb_alloc_count_free);
    }

    // TODO: add more hashmaps examples
}
