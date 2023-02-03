/*
 * The MIT License
 *
 * Copyright 2023 Andrea Vouk.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

/*------------------------------------------------------------------------------
    anv_arr (https://github.com/anvouk/anv)
--------------------------------------------------------------------------------

# anv_arr

C general purpose dynamic arrays allocated on heap.

An array self-contains within a single alloc all its elements + extra metadata
(like size, capacity, etc).

Once the array's reaches max initial capacity, the next item's insert will
automatically trigger an array expansion to accomodate the new item.

## Dependencies

- anv_metalloc.h

## Include usage

```c
// only if metalloc define is not already present somewhere else.
#define ANV_METALLOC_IMPLEMENTATION

#define ANV_ARR_IMPLEMENTATION
#include "anv_arr.h"
```

## Examples

```c
#include <stdio.h>

#define ANV_METALLOC_IMPLEMENTATION
#define ANV_ARR_IMPLEMENTATION
#include "anv_arr.h"

typedef struct item_t {
    int a;
} item_t;

int
main(void)
{
    anv_arr_t arr = anv_arr_new(10, sizeof(item_t));

    // simple push syntax
    item_t item1 = { .a = 69 };
    if (anv_arr_push(arr, &item1) != ANV_ARR_RESULT_OK) {
        fprintf(stderr, "failed inserting item\n");
    }

    // alternative push syntax
    if (anv_arr_push_new(arr, item_t, { .a = 1 }) != ANV_ARR_RESULT_OK) {
        fprintf(stderr, "failed inserting item\n");
    }

    item_t *found_item = anv_arr_get(arr, item_t, 0);
    if (!found_item) {
        fprintf(stderr, "no item at index 0 could be found\n");
    } else {
        printf("found item with value: %d\n", found_item->a);
    }

    anv_arr_destroy(arr);
}
```

------------------------------------------------------------------------------*/

#ifndef ANV_ARR_H
#define ANV_ARR_H

/*
TODO: anv_arr missing methods list
- insert (replaces existing entry at index)
- delete (moves entry to last and decrement elements count)
- delete_slow (delete entry at index and traslate all after to keep order)
- pop (return and remove last entry)
- pop_first_slow (return and remove first entry, always uses delete_slow)
- push_first (add to head and move existing at spot to last)
- push_first_slow (add to head and traslate all to keep order)
- shrink_to_fit (make capacity == length)
- sorting algorithms (narr?)
*/

#include <stddef.h> /* for size_t */

/**
 * Array status codes for operations.
 */
typedef enum anv_arr_result {
    /**
     * Operation succeeded.
     */
    ANV_ARR_RESULT_OK = 0,
    /**
     * Invalid params have been passed to method.
     * @note With debug asserts enabled, these errors always trigger an assert.
     */
    ANV_ARR_RESULT_INVALID_PARAMS = 1,
    /**
     * Memory allocation related error.
     */
    ANV_ARR_RESULT_ALLOC_ERROR = 2,
} anv_arr_result;

/**
 * Convenience alias to better recognize an anv array from a regular void *ptr;
 */
typedef void *anv_arr_t;

/**
 * Allocator callback used to determine the new size to assign to an existing
 * array when it has reached max capacity.
 *
 * Reallocator fn example:
 * @code{.c}
 * size_t
 * my_custom_reallocator(size_t old_capacity)
 * {
 *     return old_capacity + 8;
 * }
 * @endcode
 *
 * @param old_capacity The previous array's max capacity.
 * @return The new array's max capacity, which must be > old_capacity.
 */
typedef size_t (*anv_arr_reallocator_fn)(size_t old_capacity);

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Set custom reallocator callback called to determine the new array's capacity
 * to expand the array to.
 * @note A default reallocator is present by default and can be re-assigned by
 *       passing NULL as a parameter.
 * @param fn Custom reallocator fn to assign.
 */
void anv_arr_config_reallocator_fn(anv_arr_reallocator_fn fn);

/**
 * Allocate a new array with initial max capacity.
 * @param arr_capacity Initial array's max capacity, always > 0.
 * @param item_sz Size of an item to be inserted in the array.
 * @return New array, NULL on invalid params or internal alloc errors.
 */
anv_arr_t anv_arr_new(size_t arr_capacity, size_t item_sz);

/**
 * Destroy an array.
 * @note This does not clean up eventual items allocations.
 */
void anv_arr_destroy(anv_arr_t arr);

/**
 * Get number of items currently present in the array.
 * @return 0 for empty array or if cannot determine array length.
 */
size_t anv_arr_length(anv_arr_t arr);

anv_arr_result anv_arr__push(anv_arr_t *arr, void *item);

/**
 * Push a new item to the end of the array.
 * @return Status code.
 */
#define anv_arr_push(arr, item) anv_arr__push((void *)&(arr), item)

/**
 * Create a temporary struct-like item on the stack and immediately push it to
 * the end of the array.
 *
 * Example:
 * @code{.c}
 * typedef struct item_t {
 *     int a;
 * } item_t;
 *
 * anv_arr_t arr = anv_arr_new(10, sizeof(item_t));
 * anv_arr_push_new(arr, item_t, { .a = 10 });
 * anv_arr_destroy(arr);
 * @endcode
 *
 * @param type The struct-like type which will be inserted.
 * @param fields A C initialization list of the struct fields.
 * @return Status code.
 */
#define anv_arr_push_new(arr, type, fields) anv_arr_push(arr, &(type)fields)

void *anv_arr__get(anv_arr_t arr, size_t index);

/**
 * Get an item from the array at the specific index.
 *
 * Example:
 * @code{.c}
 * typedef struct item_t {
 *     int a;
 * } item_t;
 *
 * anv_arr_t arr = anv_arr_new(10, sizeof(item_t));
 * item_t *it = anv_arr_get(arr, item_t, 2);
 * anv_arr_destroy(arr);
 * @endcode
 *
 * @return NULL if no item is present at index, index exceeded array's capacity
 *         or an internal error happened.
 */
#define anv_arr_get(arr, type, index) ((type *)anv_arr__get(arr, index))

#ifdef __cplusplus
}
#endif

#ifdef ANV_ARR_IMPLEMENTATION

    #include "anv_metalloc.h"

    #ifndef anv_arr__assert
        #include <assert.h>
        #define anv_arr__assert(x) assert(x)
    #endif

    #ifdef __GNUC__
        #define ANV_ARR__LIKELY(x)   __builtin_expect((x), 1)
        #define ANV_ARR__UNLIKELY(x) __builtin_expect((x), 0)
    #else
        #define ANV_ARR__LIKELY(x)   (x)
        #define ANV_ARR__UNLIKELY(x) (x)
    #endif

typedef struct anv_arr__metadata {
    size_t arr_sz;
    size_t arr_capacity;
    size_t item_sz;
} anv_arr__metadata;

static size_t
anv_arr__reallocator_default(size_t old_capacity)
{
    return old_capacity + 8;
}

static anv_arr_reallocator_fn anv_arr__reallocator
    = anv_arr__reallocator_default;

void
anv_arr_config_reallocator_fn(anv_arr_reallocator_fn fn)
{
    if (!fn) {
        anv_arr__reallocator = anv_arr__reallocator_default;
    } else {
        anv_arr__reallocator = fn;
    }
}

anv_arr_t
anv_arr_new(size_t arr_capacity, size_t item_sz)
{
    anv_arr__metadata metadata = {
        .arr_sz = 0,
        .arr_capacity = arr_capacity,
        .item_sz = item_sz,
    };
    void *arr = anv_meta_malloc(
        &metadata, sizeof(anv_arr__metadata), item_sz * arr_capacity
    );
    return arr;
}

void
anv_arr_destroy(anv_arr_t arr)
{
    anv_meta_free(arr);
}

size_t
anv_arr_length(anv_arr_t arr)
{
    if (ANV_ARR__UNLIKELY(!arr)) {
        anv_arr__assert(0, "invalid null array");
        return 0;
    }
    anv_arr__metadata *metadata = (anv_arr__metadata *)anv_meta_get(arr);
    if (ANV_ARR__UNLIKELY(!metadata)) {
        anv_arr__assert(0, "cannot find metadata, is arr a valid meta obj?");
        return 0;
    }
    return metadata->arr_sz;
}

anv_arr_result
anv_arr__push(anv_arr_t *arr, void *item)
{
    if (ANV_ARR__UNLIKELY(!arr || !*arr)) {
        anv_arr__assert(0, "invalid null array");
        return ANV_ARR_RESULT_INVALID_PARAMS;
    }
    if (ANV_ARR__UNLIKELY(!item)) {
        anv_arr__assert(0, "invalid null item to insert");
        return ANV_ARR_RESULT_INVALID_PARAMS;
    }

    anv_arr__metadata *metadata = (anv_arr__metadata *)anv_meta_get(*arr);
    if (ANV_ARR__UNLIKELY(!metadata)) {
        anv_arr__assert(0, "cannot find metadata, is arr a valid meta obj?");
        return ANV_ARR_RESULT_INVALID_PARAMS;
    }

    if (metadata->arr_sz >= metadata->arr_capacity) {
        size_t new_capacity = anv_arr__reallocator(metadata->arr_capacity);
        void *resized_arr
            = anv_meta_realloc(*arr, metadata->item_sz * new_capacity);
        if (ANV_ARR__UNLIKELY(!resized_arr)) {
            return ANV_ARR_RESULT_ALLOC_ERROR;
        }
        metadata->arr_capacity = new_capacity;
        *arr = resized_arr;
    }

    void *spot
        = (void *)((size_t)*arr + metadata->item_sz * metadata->arr_sz++);
    memcpy(spot, item, metadata->item_sz);
    return ANV_ARR_RESULT_OK;
}

void *
anv_arr__get(anv_arr_t arr, size_t index)
{
    if (ANV_ARR__UNLIKELY(!arr)) {
        anv_arr__assert(0, "invalid null array");
        return NULL;
    }

    anv_arr__metadata *metadata = (anv_arr__metadata *)anv_meta_get(arr);
    if (ANV_ARR__UNLIKELY(!metadata)) {
        anv_arr__assert(0, "cannot find metadata, is arr a valid meta obj?");
        return NULL;
    }

    if (index >= metadata->arr_sz) {
        return NULL;
    }
    return (void *)((size_t)arr + metadata->item_sz * index);
}

#endif /* ANV_ARR_IMPLEMENTATION */

#endif /* ANV_ARR_H */
