#ifndef ANV_HALLOC_H
#define ANV_HALLOC_H

#include <stddef.h>
#include <stdint.h>

void *anvh_alloc(void *parent, size_t alloc_sz, uint16_t children_capacity);

void anvh_free(void *mem);

#ifdef ANV_HALLOC_IMPLEMENTATION

    #include <assert.h>
    #include <stdbool.h>
    #include <stdlib.h>

    /*

    void **children;
    void **parent;
    uint16_t parent_idx;
    uint16_t children_count;
    uint16_t children_capacity;
    uint16_t check_val;
    ----
    void *mem; // mem usable ptr is HERE

    */

    #define ANVH__CHECK_VAL ((uint16_t)0xFAF0)

    // remember: sizeof(void **) for the children does not exist since it's an
    // array
    #define ANVH__DATA_SZ (sizeof(uint16_t) * 4 + sizeof(void **))

    #define anvh__get_check_val(mem)                                           \
        (*((uint16_t *)((size_t)(mem) - (sizeof(uint16_t) * 1))))
    #define anvh__set_check_val(mem, val) anvh__get_check_val(mem) = (val)

    #define anvh__get_children_capacity(mem)                                   \
        (*((uint16_t *)((size_t)(mem) - (sizeof(uint16_t) * 2))))
    #define anvh__set_children_capacity(mem, val)                              \
        anvh__get_children_capacity(mem) = (val)

    #define anvh__get_children_count(mem)                                      \
        (*((uint16_t *)((size_t)(mem) - (sizeof(uint16_t) * 3))))
    #define anvh__set_children_count(mem, val)                                 \
        anvh__get_children_count(mem) = (val)

    #define anvh__get_parent_idx(mem)                                          \
        (*((uint16_t *)((size_t)(mem) - (sizeof(uint16_t) * 4))))
    #define anvh__set_parent_idx(mem, val) anvh__get_parent_idx(mem) = (val)

    #define anvh__get_parent(mem)                                              \
        (*((void **)((size_t)(mem) - (sizeof(uint16_t) * 4 + sizeof(void **)))))
    #define anvh__set_parent(mem, val) anvh__get_parent(mem) = (val)

    #define anvh__get_children(mem) ((void **)((size_t)(mem)-ANVH__DATA_SZ))

    #define anvh__get_child_at(mem, idx)                                       \
        (*(void **)((size_t)anvh__get_children(mem) - sizeof(void *) * (idx)))
    #define anvh__set_child_at(mem, idx, val)                                  \
        anvh__get_child_at(mem, idx) = (val)

static bool
anvh__attach_child(void *parent, void *child)
{
    uint16_t children_count = anvh__get_children_count(parent);
    uint16_t children_capacity = anvh__get_children_capacity(parent);
    if (children_count >= children_capacity) {
        return false;
    }

    // configure parent properties
    anvh__set_child_at(parent, children_count, child);
    anvh__set_children_count(parent, children_count + 1);

    // configure child properties
    anvh__set_parent(child, parent);
    anvh__set_parent_idx(child, children_count);
    return true;
}

void *
anvh_alloc(void *parent, size_t alloc_sz, uint16_t children_capacity)
{
    size_t data_sz = ANVH__DATA_SZ + sizeof(void *) * children_capacity;
    void *mem = calloc(1, alloc_sz + data_sz);
    if (!mem) {
        return NULL;
    }
    mem = (void *)((size_t)mem + data_sz);

    anvh__set_check_val(mem, ANVH__CHECK_VAL);
    anvh__set_children_capacity(mem, children_capacity);
    if (parent) {
        if (!anvh__attach_child(parent, mem)) {
            free((void *)((size_t)mem - data_sz));
            return NULL;
        }
    }

    return mem;
}

void
anvh_free(void *mem)
{
    if (!mem) {
        // TODO: are we ok with this?
        assert(0 && "MMMMH");
        return;
    }

    uint16_t check_val = anvh__get_check_val(mem);
    // TODO: should call regular 'free' if normal malloc has been used?
    assert(check_val == ANVH__CHECK_VAL);

    uint16_t children_count = anvh__get_children_count(mem);
    uint16_t children_capacity = anvh__get_children_capacity(mem);

    size_t children_sz = sizeof(void *) * children_capacity;
    size_t data_sz = ANVH__DATA_SZ + children_sz;

    for (uint16_t i = 0; i < children_count; ++i) {
        anvh_free(anvh__get_child_at(mem, i));
    }

    free((void *)((size_t)mem - data_sz));
}

    /*
     * Undefine macros section.
     */

    #undef ANVH__CHECK_VAL

    #undef ANVH__DATA_SZ

    #undef anvh__get_check_val
    #undef anvh__set_check_val

    #undef anvh__get_children_capacity
    #undef anvh__set_children_capacity

    #undef anvh__get_children_count
    #undef anvh__set_children_count

    #undef anvh__get_parent_idx
    #undef anvh__set_parent_idx

    #undef anvh__get_parent
    #undef anvh__set_parent

    #undef anvh__get_children

    #undef anvh__get_child_at
    #undef anvh__set_child_at

#endif /* ANV_HALLOC_IMPLEMENTATION */

#endif /* ANV_HALLOC_H */
