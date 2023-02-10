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
    anv_metalloc (https://github.com/anvouk/anv)
--------------------------------------------------------------------------------

# anv_metalloc

Store 'hidden' metadata for allocated memory right next to it.

Advantages:
- 'hide' memory block related info (e.g. its size, etc.)
- 1 allocation per memory block + its metadatas
- minimal overhead during allocations

Drawbacks:
- allocation size is larger (default min is at least 5+ bytes)

```
== brief overview ==

allocated memory block looks like this:

|-----metadata---sc|-------------------data------------------|   ... ->
                 ^ptr points here

where:
s = stores metadata size (s and c excluded. default max is 256).
  retrieve with anv_meta_getsz()
  see ANV_METALLOC_METASIZE).
c = check byte (default is 4 bytes).
  used to check metallocation validity (see anv_meta_isvalid())
  see CHKB.

to sum it up the default total allocation size is:
metadata size + metadata size num size + check byte size + memory allocated
which translates to:
metadata size + METASZ_SZ + CHKB_SZ + memory allocated   bytes
...           + 1         + 4       + ...                bytes
```

## Dependencies

None

## Include usage

```c
#define ANV_METALLOC_IMPLEMENTATION
#include "anv_metalloc.h"
```

## Examples

### Simple example

```c
#define ANV_METALLOC_IMPLEMENTATION
#include "anv_metalloc.h"

int main(void)
{
    int metaval = 20;
    int *int_arr = anv_meta_malloc(
        &metaval,
        sizeof(metaval),
        sizeof(int) * 10
    );

    // int_arr works like any other dynamic array

    // retrieve metadata
    printf("metavalue is: %d\n", *(int *)anv_meta_get(int_arr));

    // change metadata value
    metaval = 30;
    if (anv_meta_set(int_arr, &metaval) != ANV_META_RESULT_OK) {
        fprintf(stderr, "failed setting metadata\n");
        goto done;
    }

    printf("metavalue is: %d\n", *(int *)anv_meta_get(int_arr));

done:
    anv_meta_free(int_arr); // very important!
}
```

------------------------------------------------------------------------------*/

#ifndef ANV_METALLOC_H
#define ANV_METALLOC_H

#include <stddef.h> /* for size_t, ptrdiff_t */

#ifndef ANV_METALLOC_METASIZE
    #define ANV_METALLOC_METASIZE unsigned char
#endif

typedef ANV_METALLOC_METASIZE anv_meta_size_t;

/**
 * Metalloc status codes for operations.
 */
typedef enum anv_meta_result {
    /**
     * Operation succeeded.
     */
    ANV_META_RESULT_OK = 0,
    /**
     * Invalid params have been passed to method.
     * @note With debug asserts enabled, these errors always trigger an assert.
     */
    ANV_META_RESULT_INVALID_PARAMS = 1,
} anv_meta_result;

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Check whether mem object is a valid metallocated object and not null.
 * @param mem Metallocated memory block to check.
 * @return 1 if valid, 0 otherwise.
 */
int anv_meta_isvalid(void *mem);

/**
 * Get metadata size for passed memory object.
 * @param mem Metallocated memory block.
 * @return metadata size, always > 0.
 */
anv_meta_size_t anv_meta_getsz(void *mem);

/**
 * Get metadata for passes memory object.
 * @param mem Metallocated memory block.
 * @return Fully zeroed metadata object if metadata was set to NULL.
 */
void *anv_meta_get(void *mem);

/**
 * Change metadata for passed memory object.
 * @param mem Metallocated memory block.
 * @param metadata New metadata value. Must be of correct size specified at
 *        malloc time.
 * @return Status code.
 */
anv_meta_result anv_meta_set(void *mem, void *metadata);

/**
 * Get offset between real memory allocated on heap and first data byte after
 * the metadata.
 * @note subtracting this size from mem you get the pointer to the heap object
 *       passed to regular malloc and thus freeable with regular free.
 * @param mem Metallocated memory block.
 */
ptrdiff_t anv_meta_get_offset(void *mem);

/**
 * Allocate on the heap a new matallocated object.
 *
 * A metallocated object is malloc'd object plus a metadata part before. There
 * is small memory overhead for each object but this allows the storage of
 * custom data for the object while allowing the same usages as a regular
 * malloc'd object (for the most part).
 *
 * @param metadata Optional metadata value to store. Can always be set later.
 * @param meta_sz Size of the metadata value to store. Can not be zero and must
 *                not exceed sizeof(anv_meta_size_t).
 * @param data_sz Size of the data portion to allocated
 * @return A pointer to the data portion of the object and after the metadata
 *         part. To manage the metadata part, use the corresponding methods in
 *         this lib.
 */
void *anv_meta_malloc(void *metadata, anv_meta_size_t meta_sz, size_t data_sz);

/**
 * Free metallocated object memory.
 * @param mem Metallocated memory block, passing NULL is safe and does nothing.
 */
void anv_meta_free(void *mem);

/**
 * Reallocate metallocated data memory part.
 * @note This method does not change size of the metadata portion.
 * @param mem Metallocated memory block.
 * @param new_sz New size of the data portion.
 * @return Pointer to data portion if succeeded.
 */
void *anv_meta_realloc(void *mem, size_t new_sz);

#ifdef __cplusplus
}
#endif

#ifdef ANV_METALLOC_IMPLEMENTATION

    #include <stdint.h> /* for uint32_t */
    #include <stdlib.h> /* for malloc(), ... */
    #include <string.h> /* for memset() */

    #ifndef anv_meta__assert
        #include <assert.h>
        #define anv_meta__assert(cond, errmsg) assert((cond) && (errmsg))
    #endif

    #ifdef __GNUC__
        #define ANV_META__LIKELY(x)   __builtin_expect((x), 1)
        #define ANV_META__UNLIKELY(x) __builtin_expect((x), 0)
    #else
        #define ANV_META__LIKELY(x)   (x)
        #define ANV_META__UNLIKELY(x) (x)
    #endif

typedef uint32_t chkb_t;

    #define CHKB ((chkb_t)0x696941469)

    #define METASZ_SZ sizeof(anv_meta_size_t)
    #define CHKB_SZ   sizeof(chkb_t)

int
anv_meta_isvalid(void *mem)
{
    if (!mem) {
        return 0;
    }
    return *(chkb_t *)((size_t)mem - CHKB_SZ) == CHKB ? 1 : 0;
}

static anv_meta_size_t
anv_meta__getsz(void *mem)
{
    return *(anv_meta_size_t *)((size_t)mem - METASZ_SZ - CHKB_SZ);
}

anv_meta_size_t
anv_meta_getsz(void *mem)
{
    if (ANV_META__UNLIKELY(!anv_meta_isvalid(mem))) {
        anv_meta__assert(0, "not a valid metallocated object");
        return 0;
    }
    return anv_meta__getsz(mem);
}

void *
anv_meta_get(void *mem)
{
    if (ANV_META__UNLIKELY(!anv_meta_isvalid(mem))) {
        anv_meta__assert(0, "not a valid metallocated object");
        return NULL;
    }
    return (void *)((size_t)mem - anv_meta__getsz(mem) - METASZ_SZ - CHKB_SZ);
}

static void
anv_meta__set(void *full_mem, void *metadata, size_t meta_sz)
{
    if (metadata) {
        memcpy(full_mem, metadata, meta_sz);
    } else {
        memset(full_mem, 0, meta_sz);
    }
}

anv_meta_result
anv_meta_set(void *mem, void *metadata)
{
    if (ANV_META__UNLIKELY(!anv_meta_isvalid(mem))) {
        anv_meta__assert(0, "not a valid metallocated object");
        return ANV_META_RESULT_INVALID_PARAMS;
    }

    anv_meta_size_t meta_sz = anv_meta__getsz(mem);
    void *full_mem = (void *)((size_t)mem - meta_sz - METASZ_SZ - CHKB_SZ);

    anv_meta__set(full_mem, metadata, meta_sz);
    return ANV_META_RESULT_OK;
}

static ptrdiff_t
anv_meta_get__offset(void *mem)
{
    return (ptrdiff_t)(anv_meta__getsz(mem) + METASZ_SZ + CHKB_SZ);
}

ptrdiff_t
anv_meta_get_offset(void *mem)
{
    if (ANV_META__UNLIKELY(!anv_meta_isvalid(mem))) {
        anv_meta__assert(0, "not a valid metallocated object");
        return 0;
    }
    return anv_meta_get__offset(mem);
}

void *
anv_meta_malloc(void *metadata, anv_meta_size_t meta_sz, size_t data_sz)
{
    if (ANV_META__UNLIKELY(data_sz <= 0)) {
        anv_meta__assert(0, "trying to allocate 0 bytes is not supported");
        return NULL;
    }
    if (ANV_META__UNLIKELY(meta_sz <= 0)) {
        anv_meta__assert(0, "metadata allocation size cannot be 0");
        return NULL;
    }

    void *full_mem = malloc(data_sz + meta_sz + METASZ_SZ + CHKB_SZ);
    if (ANV_META__UNLIKELY(!full_mem)) {
        return NULL;
    }

    anv_meta__set(full_mem, metadata, meta_sz);

    *(anv_meta_size_t *)((size_t)full_mem + meta_sz) = meta_sz;
    *(chkb_t *)((size_t)full_mem + meta_sz + METASZ_SZ) = CHKB;
    return (void *)((size_t)full_mem + meta_sz + METASZ_SZ + CHKB_SZ);
}

void
anv_meta_free(void *mem)
{
    if (ANV_META__UNLIKELY(!anv_meta_isvalid(mem))) {
        anv_meta__assert(0, "not a valid metallocated object");
        return;
    }
    free((void *)((size_t)mem - (anv_meta__getsz(mem) + METASZ_SZ + CHKB_SZ)));
}

void *
anv_meta_realloc(void *mem, size_t new_sz)
{
    if (ANV_META__UNLIKELY(!anv_meta_isvalid(mem))) {
        anv_meta__assert(0, "not a valid metallocated object");
        return NULL;
    }

    ptrdiff_t padd = anv_meta_get__offset(mem);
    void *full_mem = (void *)((size_t)mem - padd);

    anv_meta_size_t meta_sz = anv_meta__getsz(mem);

    void *reallocated_mem
        = realloc(full_mem, new_sz + meta_sz + METASZ_SZ + CHKB_SZ);
    if (ANV_META__UNLIKELY(!reallocated_mem)) {
        free(full_mem);
        return NULL;
    }

    return (void *)((size_t)reallocated_mem + padd);
}

#endif /* ANV_METALLOC_IMPLEMENTATION */

#endif /* ANV_METALLOC_H */
