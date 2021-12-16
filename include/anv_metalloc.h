/*
 * The MIT License
 *
 * Copyright 2019 Andrea Vouk.
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
    anv_metalloc
--------------------------------------------------------------------------------

  store 'hidden' metadata for allocated memory right next to it.

  advantages:
  - 'hide' memory block related info (e.g. its size, etc.)
  - 1 allocation per memory block + its metadatas
  - minimal overhead during allocations

  drawbacks:
  - allocation size is larger (default at least 5+ bytes)
  - extra care must be taken for arrays of metallocated memory blocks

  == brief overview ==

  allocated memory block looks like this:

  |-----metadata---sc|-------------------data------------------|   ... ->
                     ^ptr points here

  where:
  s = stores metadata size (s and c excluded. default max is 256)
      retrieve with anv_meta_getsz()
      see ANV_METALLOC_METASIZE)
  c = check byte (default is 4 bytes)
      used to check metallocation validity (see anv_meta_isvalid())
      see CHKB

  to sum it up the default total allocation size is:
    metadata size + metadata size num size + check byte size + memory allocated
  which translates to:
    metadata size + METASZ_SZ + CHKB_SZ + memory allocated   bytes
    ...           + 1         + 4       + ...                bytes

  TODO:
  - add example of arrays of metallocated objects.

  simple example:

    #define ANV_METALLOC_IMPLEMENTATION
    #include "anv_metalloc.h"

    int main(void)
    {
        int metaval = 20;
        int *int_arr = anv_meta_malloc(&metaval, sizeof(metaval), sizeof(int) * 10);
        // int_arr works like any other dynamic array

        // retrieve metadata
        printf("metavalue is: %d\n", *(int *)anv_meta_get(int_arr));

        // change metadata value
        metaval = 30;
        anv_meta_set(int_arr, &metaval);

        printf("metavalue is: %d\n", *(int *)anv_meta_get(int_arr));

        anv_meta_free(int_arr); // very important!
    }

------------------------------------------------------------------------------*/

#ifndef ANV_METALLOC_H
#define ANV_METALLOC_H

#include <stddef.h> /* for size_t */

#ifndef ANV_METALLOC_EXP
#  define ANV_METALLOC_EXP extern 
#endif

#ifndef ANV_METALLOC_METASIZE
#  define ANV_METALLOC_METASIZE unsigned char
#endif

typedef ANV_METALLOC_METASIZE anv_meta_size_t;

ANV_METALLOC_EXP int anv_meta_isvalid(void *mem);

ANV_METALLOC_EXP anv_meta_size_t anv_meta_getsz(void *mem);
ANV_METALLOC_EXP void *anv_meta_get(void *mem);
ANV_METALLOC_EXP void anv_meta_set(void *mem, void *metadata);

ANV_METALLOC_EXP size_t anv_meta_getpadding(void *mem);

ANV_METALLOC_EXP void *anv_meta_malloc(void *metadata, anv_meta_size_t data_sz, size_t mem_sz);
ANV_METALLOC_EXP void *anv_meta_realloc(void *mem, size_t new_sz);
ANV_METALLOC_EXP void *anv_meta_calloc(void *metadata, anv_meta_size_t data_sz, size_t nitems, size_t size);
ANV_METALLOC_EXP void anv_meta_free(void *mem);

#ifdef ANV_METALLOC_IMPLEMENTATION

#include <stdlib.h> /* for malloc(), ... */
#include <string.h> /* for memset() */
#include <stdint.h> /* for uint32_t */

#ifndef anv_meta__assert
#  include <assert.h>
#  define anv_meta__assert(x) assert(x)
#endif

#define CHKB       0x12345678
#define CHKB_T     uint32_t

typedef CHKB_T     chkb_t;

#define METASZ_SZ  sizeof(anv_meta_size_t)
#define CHKB_SZ    sizeof(chkb_t)

int
anv_meta_isvalid(void *mem)
{
    return *(chkb_t *)(mem - CHKB_SZ) == (chkb_t)CHKB ? 1 : 0;
}

anv_meta_size_t
anv_meta_getsz(void *mem)
{
    /* return metadata size */
    anv_meta__assert(anv_meta_isvalid(mem));
#ifdef ANV_METALLOC_ENABLE_EXTRA_CHECKS
    if (!anv_meta_isvalid(mem))
        return 0;
#endif
    return *(anv_meta_size_t *)(mem - (METASZ_SZ + CHKB_SZ));
}

void *
anv_meta_get(void *mem)
{
    anv_meta__assert(anv_meta_isvalid(mem));
#ifdef ANV_METALLOC_ENABLE_EXTRA_CHECKS
    if (!anv_meta_isvalid(mem))
        return NULL;
#endif
    return (mem - (anv_meta_getsz(mem) + METASZ_SZ + CHKB_SZ));
}

void
anv_meta_set(void *mem, void *metadata)
{
    anv_meta__assert(anv_meta_isvalid(mem));
#ifdef ANV_METALLOC_ENABLE_EXTRA_CHECKS
    if (!anv_meta_isvalid(mem))
        return;
#endif
    anv_meta_size_t data_sz = anv_meta_getsz(mem);
    memcpy((mem - (data_sz + METASZ_SZ + CHKB_SZ)), metadata, data_sz);
}

size_t
anv_meta_getpadding(void *mem)
{
    anv_meta__assert(anv_meta_isvalid(mem));
#ifdef ANV_METALLOC_ENABLE_EXTRA_CHECKS
    if (!anv_meta_isvalid(mem))
        return 0;
#endif
    return (size_t)(anv_meta_getsz(mem) + METASZ_SZ + CHKB_SZ);
}

void *
anv_meta_malloc(void *metadata, anv_meta_size_t data_sz, size_t mem_sz)
{
    void *mem = malloc(mem_sz + CHKB_SZ + data_sz);
    if (!mem)
        return NULL;
    if (metadata)
        memcpy(mem, metadata, data_sz);
    else
        memset(mem, 0, data_sz);
    *(anv_meta_size_t *)(mem + data_sz   ) = data_sz;
    *(chkb_t *)(mem + data_sz + METASZ_SZ) = CHKB;
    return (mem + data_sz + METASZ_SZ + CHKB_SZ);
}

void *
anv_meta_realloc(void *mem, size_t new_sz)
{
    anv_meta__assert(anv_meta_isvalid(mem));
    anv_meta__assert(mem && "use meta_malloc for init and meta_realloc for resize only");
#ifdef ANV_METALLOC_ENABLE_EXTRA_CHECKS
    if (!anv_meta_isvalid(mem))
        return NULL;
#endif
    size_t padd = anv_meta_getpadding(mem);
    void *actual_mem = (mem - padd);
    actual_mem = realloc(actual_mem, new_sz);
    if (!actual_mem)
        return NULL;
    return (actual_mem + padd);
}

void *
anv_meta_calloc(void *metadata, anv_meta_size_t data_sz, size_t nitems, size_t size)
{
    /* not really using calloc here :) */
    size_t mem_sz = nitems * size;
    void *mem = anv_meta_malloc(metadata, data_sz, mem_sz);
    if (!mem)
        return NULL;
    size_t padd = anv_meta_getpadding(mem);
    memset(mem - padd, mem_sz + padd, 0);
    return mem;
}

void
anv_meta_free(void *mem)
{
    if (!anv_meta_isvalid(mem))
        return free(mem);
    free(mem - (anv_meta_getsz(mem) + METASZ_SZ + CHKB_SZ));
}

#endif

#endif /* ANV_METALLOC_H */
