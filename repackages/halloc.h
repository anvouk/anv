/*
 *	This file is a part of Hierarchical Allocator library.
 *	Copyright (c) 2004-2011 Alex Pankratov. All rights reserved.
 *
 *	http://swapped.cc/halloc
 */
/*
 *  Repackaged as a single header C lib and cleaned up by Andrea Vouk.
 *  Other changes:
 *  - Remove reliance on realloc to free stuff (now always uses free).
 *  - Rename max_align_t to h_max_align_t to prevent conflicts on Linux.
 *  - Rename realloc_t to h_realloc_t to prevent conflicts.
 *  - Add C++ extern C wrapper.
 *
 *  https://github.com/anvouk/anv
 */

/*
 *	The program is distributed under terms of BSD license.
 *	You can obtain the copy of the license by visiting:
 *
 *	http://www.opensource.org/licenses/bsd-license.php
 */

#ifndef _LIBP_HALLOC_H_
#define _LIBP_HALLOC_H_

#include <stddef.h> /* size_t */

#ifdef __cplusplus
extern "C" {
#endif

/*
 * core API
 */
void *halloc(void *block, size_t len);
void hattach(void *block, void *parent);

/*
 * standard malloc/free api
 */
void *h_malloc(size_t len);
void *h_calloc(size_t n, size_t len);
void *h_realloc(void *p, size_t len);
void h_free(void *p);
char *h_strdup(const char *str);

#ifdef __cplusplus
}
#endif

/*
 * the underlying allocator
 */
typedef void *(*h_realloc_t)(void *ptr, size_t len);

extern h_realloc_t halloc_allocator;

#ifdef HALLOC_IMPLEMENTATION

#include <assert.h> /* asssert */
#include <stdlib.h> /* realloc */
#include <string.h> /* memset & co */

/*
 * restore pointer to the structure by a pointer to its field
 */
#define structof(p, t, f) ((t *)(-offsetof(t, f) + (size_t)(void *)(p)))

/*
 * redefine for the target compiler
 */
#define static_inline static __inline__

/*
 * weak double-linked list w/ tail sentinel
 */
typedef struct hlist_head hlist_head_t;
typedef struct hlist_item hlist_item_t;

struct hlist_head {
    hlist_item_t *next;
};

struct hlist_item {
    hlist_item_t *next;
    hlist_item_t **prev;
};

/*
 * shared tail sentinel
 */
struct hlist_item hlist_null;

#define __hlist_init(h)                                                        \
    {                                                                          \
        &hlist_null                                                            \
    }

#define __hlist_init_item(i)                                                   \
    {                                                                          \
        &hlist_null, &(i).next                                                 \
    }

static_inline void hlist_init(hlist_head_t *h);
static_inline void hlist_init_item(hlist_item_t *i);

static_inline int hlist_item_listed(hlist_item_t *i);

static_inline void hlist_add(hlist_head_t *h, hlist_item_t *i);
static_inline void hlist_del(hlist_item_t *i);

static_inline void hlist_relink(hlist_item_t *i);
static_inline void hlist_relink_head(hlist_head_t *h);

#define hlist_for_each(i, h) for (i = (h)->next; i != &hlist_null; i = i->next)

#define hlist_for_each_safe(i, tmp, h)                                         \
    for (i = (h)->next, tmp = i->next; i != &hlist_null; i = tmp, tmp = i->next)

/*
 * static
 */
static_inline void
hlist_init(hlist_head_t *h)
{
    assert(h);
    h->next = &hlist_null;
}

static_inline void
hlist_init_item(hlist_item_t *i)
{
    assert(i);
    i->prev = &i->next;
    i->next = &hlist_null;
}

static_inline int
hlist_item_listed(hlist_item_t *i)
{
    assert(i);
    return i->prev != &i->next;
}

static_inline void
hlist_add(hlist_head_t *h, hlist_item_t *i)
{
    hlist_item_t *next;
    assert(h && i);

    next = i->next = h->next;
    next->prev = &i->next;
    h->next = i;
    i->prev = &h->next;
}

static_inline void
hlist_del(hlist_item_t *i)
{
    hlist_item_t *next;
    assert(i);

    next = i->next;
    next->prev = i->prev;
    *i->prev = next;

    hlist_init_item(i);
}

static_inline void
hlist_relink(hlist_item_t *i)
{
    assert(i);
    *i->prev = i;
    i->next->prev = &i->next;
}

static_inline void
hlist_relink_head(hlist_head_t *h)
{
    assert(h);
    h->next->prev = &h->next;
}

/*
 * a type with the most strict alignment requirements
 */
typedef union h_max_align {
    char c;
    short s;
    long l;
    int i;
    float f;
    double d;
    void *v;
    void (*q)(void);
} h_max_align_t;

/*
 * block control header
 */
typedef struct hblock {
#ifndef NDEBUG
#define HH_MAGIC 0x20040518L
    long magic;
#endif
    hlist_item_t siblings; /* 2 pointers */
    hlist_head_t children; /* 1 pointer  */
    h_max_align_t data[1]; /* not allocated, see below */
} hblock_t;

h_realloc_t halloc_allocator = NULL;

#define sizeof_hblock offsetof(hblock_t, data)

#define allocator halloc_allocator

/*
 * static methods
 */
static int _ok_to_multiply(size_t a, size_t b);

static void _set_allocator(void);
static void *_realloc(void *ptr, size_t n);

static int _relate(hblock_t *b, hblock_t *p);
static void _free_children(hblock_t *p);

/*
 * core API
 */
void *
halloc(void *ptr, size_t len)
{
    hblock_t *p;

    /* set up default allocator */
    if (!allocator) {
        _set_allocator();
        assert(allocator);
    }

    /* a quick overflow check */
    if (len + sizeof_hblock < sizeof_hblock) {
        return NULL;
    }

    /* calloc */
    if (!ptr) {
        if (!len) {
            return NULL;
        }

        p = allocator(0, len + sizeof_hblock);
        if (!p) {
            return NULL;
        }
#ifndef NDEBUG
        p->magic = HH_MAGIC;
#endif
        hlist_init(&p->children);
        hlist_init_item(&p->siblings);

        return p->data;
    }

    p = structof(ptr, hblock_t, data);
    assert(p->magic == HH_MAGIC);

    /* realloc */
    if (len) {
        int listed = hlist_item_listed(&p->siblings);

        p = allocator(p, len + sizeof_hblock);
        if (!p) {
            return NULL;
        }

        hlist_relink_head(&p->children);

        if (listed) {
            hlist_relink(&p->siblings);
        } else {
            hlist_init_item(&p->siblings);
        }

        return p->data;
    }

    /* free */
    _free_children(p);
    hlist_del(&p->siblings);
    allocator(p, 0);

    return NULL;
}

void
hattach(void *block, void *parent)
{
    hblock_t *b, *p;

    if (!block) {
        assert(!parent);
        return;
    }

    /* detach */
    b = structof(block, hblock_t, data);
    assert(b->magic == HH_MAGIC);

    hlist_del(&b->siblings);

    if (!parent) {
        return;
    }

    /* attach */
    p = structof(parent, hblock_t, data);
    assert(p->magic == HH_MAGIC);

    /* sanity checks */
    assert(b != p); /* trivial */
    assert(!_relate(p, b)); /* heavy ! */

    hlist_add(&p->children, &b->siblings);
}

/*
 * malloc/free api
 */
void *
h_malloc(size_t len)
{
    return halloc(0, len);
}

void *
h_calloc(size_t n, size_t len)
{
    void *ptr;

    if (!_ok_to_multiply(n, len)) {
        return NULL;
    }

    ptr = halloc(0, len *= n);
    return ptr ? memset(ptr, 0, len) : NULL;
}

void *
h_realloc(void *ptr, size_t len)
{
    return halloc(ptr, len);
}

void
h_free(void *ptr)
{
    halloc(ptr, 0);
}

char *
h_strdup(const char *str)
{
    size_t len = strlen(str);
    char *ptr = halloc(0, len + 1);
    return ptr ? (ptr[len] = 0, memcpy(ptr, str, len)) : NULL;
}

/*
 * static stuff
 */
#define SIZE_T_MAX  ((size_t)-1)
#define SIZE_T_HALF (((size_t)1) << 4 * sizeof(size_t))

static int
_ok_to_multiply(size_t a, size_t b)
{
    return (a < SIZE_T_HALF && b < SIZE_T_HALF) || (a == 0)
        || (SIZE_T_MAX / a < b);
}

static void
_set_allocator(void)
{
    void *p;
    assert(!allocator);

    /*
     * Do not try to rely on realloc to free memory even if it supported.
     * The _realloc wrapper does a good job anyway without really any penalty
     * so let's not do any 'clever' stuff where it's not necesseary.
     * A.V.
     */
    allocator = _realloc;
}

static void *
_realloc(void *ptr, size_t n)
{
    /*
     * free'ing realloc()
     */
    if (n) {
        return realloc(ptr, n);
    }
    free(ptr);
    return NULL;
}

static int
_relate(hblock_t *b, hblock_t *p)
{
    hlist_item_t *i;

    if (!b || !p) {
        return 0;
    }

    /*
     * since there is no 'parent' pointer, which would've allowed
     * O(log(n)) upward traversal, the check must use O(n) downward
     * iteration of the entire hierarchy; and this can be VERY SLOW
     */
    hlist_for_each(i, &p->children)
    {
        hblock_t *q = structof(i, hblock_t, siblings);
        if (q == b || _relate(b, q)) {
            return 1;
        }
    }
    return 0;
}

static void
_free_children(hblock_t *p)
{
    hlist_item_t *i, *tmp;

#ifndef NDEBUG
    /*
     * this catches loops in hierarchy with almost zero
     * overhead (compared to _relate() running time)
     */
    assert(p && p->magic == HH_MAGIC);
    p->magic = 0;
#endif
    hlist_for_each_safe(i, tmp, &p->children)
    {
        hblock_t *q = structof(i, hblock_t, siblings);
        _free_children(q);
        allocator(q, 0);
    }
}

#undef structof
#undef static_inline

#undef __hlist_init
#undef __hlist_init_item

#undef hlist_for_each
#undef hlist_for_each_safe

#undef HH_MAGIC
#undef sizeof_hblock
#undef allocator

#undef SIZE_T_MAX
#undef SIZE_T_HALF

#endif /* HALLOC_IMPLEMENTATION */

#endif /* _LIBP_HALLOC_H_ */
