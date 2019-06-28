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
    anv_leaks
--------------------------------------------------------------------------------

  memory leaks (and other bugs) spotter.

  keeps track of malloc(), free(), calloc() and realloc() calls count and of what
  happens to the allocated memory.

  minimalist example:

    #define ANV_LEAKS_IMPLEMENTATION
    #include <anv_leaks.h>

    #define malloc      anv_leaks_malloc
    #define free        anv_leaks_free
    #define calloc      anv_leaks_calloc
    #define realloc     anv_leaks_realloc

    int main(void)
    {
        anv_leaks_init(stdout);
        void *mem = malloc(10);
        anv_leaks_quickpeek();
        free(mem);
        anv_leaks_quickpeek();
    }

  use anv_leaks_get_stats(), anv_leaks_get_leaks() and
  anv_leaks_free_info() to customize how to show memory info and stats.

------------------------------------------------------------------------------*/

#ifndef ANV_LEAKS_H
#define ANV_LEAKS_H

#include <stdio.h>

#ifndef ANVLKAPI
#  define ANVLKAPI extern
#endif

typedef struct anv_leak_info {
    const char *filename;
    int line;
    size_t bytes;
    void *address;
} anv_leak_info;

typedef struct anv_leaks_stats {
    unsigned total_allocated;
    unsigned total_freed;
    unsigned malloc_count;
    unsigned free_count;
    unsigned calloc_count;
    unsigned realloc_count;
} anv_leaks_stats;

ANVLKAPI void anv_leaks_init(FILE *output);
ANVLKAPI void anv_leaks_quickpeek(void);

ANVLKAPI void anv_leaks_get_stats(anv_leaks_stats *out);
ANVLKAPI void anv_leaks_get_leaks(anv_leak_info  ***out_arr, size_t *out_sz);
ANVLKAPI void anv_leaks_free_info(anv_leak_info **leak_arr, size_t sz);

ANVLKAPI void *anv_leaks_malloc_(size_t size, const char *filename, int line, int is_realloc);
ANVLKAPI void  anv_leaks_free_(void *mem, const char *filename, int line);
ANVLKAPI void *anv_leaks_calloc_(size_t num, size_t size, const char *filename, int line);
ANVLKAPI void *anv_leaks_realloc_(void *mem, size_t size, const char *filename, int line);

#define anv_leaks_malloc(size)       anv_leaks_malloc_ (size, __FILE__, __LINE__, 0)
#define anv_leaks_free(mem)          anv_leaks_free_   (mem, __FILE__, __LINE__)
#define anv_leaks_calloc(num, size)  anv_leaks_calloc_ (num, size, __FILE__, __LINE__)
#define anv_leaks_realloc(mem, size) anv_leaks_realloc_(mem, size, __FILE__, __LINE__)

#ifndef ANV_LEAKS_DISABLE
#  define malloc      anv_leaks_malloc
#  define free        anv_leaks_free
#  define calloc      anv_leaks_calloc
#  define realloc     anv_leaks_realloc
#endif

#ifdef ANV_LEAKS_IMPLEMENTATION

#ifndef anv_leaks__assert
#  include <assert.h>
#  define anv_leaks__assert(x) assert(x)
#endif

#include <stdlib.h>
#include <string.h>

typedef struct anv_leaks__alloc_map {
    void *block;
    size_t size;
    const char *filename;
    int line;
    struct anv_leaks__alloc_map *next;
    struct anv_leaks__alloc_map *prev;
} anv_leaks__alloc_map;

static void
anv_leaks__map_init(anv_leaks__alloc_map *head)
{
    head->next = head;
    head->prev = head;
}

static void
anv_leaks__map_add(anv_leaks__alloc_map *head, anv_leaks__alloc_map *new_list)
{
    head->next->prev = new_list;
    new_list->next = head->next;
    new_list->prev = head;
    head->next = new_list;
}

static void
anv_leaks__map_add_tail(anv_leaks__alloc_map *head, anv_leaks__alloc_map *new_list)
{
    head->prev = new_list;
    new_list->next = head;
    new_list->prev = head->prev;
    head->prev->next = new_list;
}

static void
anv_leaks__map_delete(anv_leaks__alloc_map *entry)
{
    entry->next->prev = entry->prev;
    entry->prev->next = entry->next;
    entry->next = NULL;
    entry->prev = NULL;
}

static void
anv_leaks__map_move(anv_leaks__alloc_map *head, anv_leaks__alloc_map *list)
{
    list->next->prev = list->prev;
    list->prev->next = list->next;
    anv_leaks__map_add(head, list);
}

static void
anv_leaks__map_move_tail(anv_leaks__alloc_map *head, anv_leaks__alloc_map *list)
{
    list->next->prev = list->prev;
    list->prev->next = list->next;
    anv_leaks__map_add_tail(head, list);
}

static void
anv_leaks__map_splice(anv_leaks__alloc_map *head, anv_leaks__alloc_map *list)
{
    if (list->next != list) {
        anv_leaks__alloc_map *first = list->next;
        anv_leaks__alloc_map *last = list->prev;
        anv_leaks__alloc_map *at = head->next;
        first->prev = head;
        head->next = first;
        last->next = at;
        at->prev = last;
    }
}

/*
#define anv_leaks__map_is_empty(head) \
    ((head)->next == (head))
*/

#define anv_leaks__map_foreach(head, pos) \
	for (pos = (head)->next; pos != (head); pos = pos->next)

/*
#define anv_leaks__map_foreach_backward(head, pos) \
    for (pos = (head)->prev; pos != (head); pos = pos->prev)

#define anv_leaks__map_foreach_safe(head, pos, n) \
    for (pos = (head)->next, n = pos->next; pos != (head); pos = n, n = pos->next)

#define anv_leaks__map_foreach_safe_backward(head, pos, n) \
    for (pos = (head)->prev, n = pos->prev; pos != (head); pos = n, n = pos->prev)
*/

static struct {
    FILE *output;
    anv_leaks_stats info;
    size_t sz;
    anv_leaks__alloc_map map;
} anv_leaks__settings;

void
anv_leaks_init(FILE *output)
{
    anv_leaks__assert(output);
    anv_leaks__settings.output = output;
    anv_leaks__settings.map.block = NULL;
    anv_leaks__settings.map.size = 0;
    anv_leaks__map_init(&anv_leaks__settings.map);
    memset(&anv_leaks__settings.info, 0, sizeof(anv_leaks_stats));
}

void
anv_leaks_quickpeek(void)
{
    const char *msg =
        "\n /=========================\\\n"
        " |===    Quick Stats    ===|\n"
        " |=========================|\n"
        " |total alloc:      %.7d|\n"
        " |total free:       %.7d|\n"
        " |-------------------------|\n"
        " |total leaks:      %.7d|\n"
        " |                         |\n"
        " |total malloc():   %.7d|\n"
        " |total calloc():   %.7d|\n"
        " |-------------------------|\n"
        " |total free():     %.7d|\n"
        " |                         |\n"
        " |total realloc():  %.7d|\n"
        " \\=========================/\n\n";

    fprintf(anv_leaks__settings.output, msg,
        anv_leaks__settings.info.total_allocated,
        anv_leaks__settings.info.total_freed,
        /*-------------------------*/
        anv_leaks__settings.info.total_allocated - anv_leaks__settings.info.total_freed,
        /*                         */
        anv_leaks__settings.info.malloc_count,
        anv_leaks__settings.info.calloc_count,
        /*-------------------------*/
        anv_leaks__settings.info.free_count,
        /*                         */
        anv_leaks__settings.info.realloc_count
    );
}

void
anv_leaks_get_stats(anv_leaks_stats *out)
{
    anv_leaks__assert(out);
    out->total_allocated = anv_leaks__settings.info.total_allocated;
    out->total_freed = anv_leaks__settings.info.total_freed;
    out->malloc_count = anv_leaks__settings.info.malloc_count;
    out->free_count = anv_leaks__settings.info.free_count;
    out->calloc_count = anv_leaks__settings.info.calloc_count;
    out->realloc_count = anv_leaks__settings.info.realloc_count;
}

void
anv_leaks_get_leaks(anv_leak_info ***out_arr, size_t *out_sz)
{
    size_t delta, i;
    anv_leaks__alloc_map *il;
    anv_leak_info *leak;

    anv_leaks__assert(out_arr);
    anv_leaks__assert(out_sz);

    delta = anv_leaks__settings.info.malloc_count + anv_leaks__settings.info.calloc_count
        - anv_leaks__settings.info.free_count;
    if (delta == 0)
        goto anv__zero_all;
    *out_arr = malloc(sizeof(anv_leak_info*) * delta);
    anv_leaks__assert(*out_arr);
    i = 0;
    anv_leaks__map_foreach(&anv_leaks__settings.map, il) {
        leak = malloc(sizeof(anv_leak_info));
        anv_leaks__assert(leak);
        leak->filename = il->filename;
        leak->line = il->line;
        leak->bytes = il->size;
        leak->address = il->block;
        (*out_arr)[i] = leak;
        ++i;
    }
    *out_sz = delta;
    return;

anv__zero_all:
    *out_arr = NULL;
    *out_sz = 0;
}

void
anv_leaks_free_info(anv_leak_info **leak_arr, size_t sz)
{
    size_t i;

    if (!leak_arr || sz == 0)
        return;
    for (i = 0; i < sz; ++i) {
        free(leak_arr[i]);
    }
    free(leak_arr);
    *leak_arr = NULL;
}

void *
anv_leaks_malloc_(size_t size, const char *filename, int line, int is_realloc)
{
    void* mem;
    anv_leaks__alloc_map* node;

    anv_leaks__assert(size != 0);

    /* alloc new node */
    node = malloc(sizeof(anv_leaks__alloc_map));
    anv_leaks__assert(node);
    mem = malloc(size);
    if (!mem) {
        /* check if realloc is calling malloc */
        if (is_realloc)
            anv_leaks__assert(0 && "(realloc) malloc failed.");
        else
            anv_leaks__assert(0 && "malloc failed.");
        return NULL;
    }
    /* intialize new node */
    node->block = mem;
    node->size = size;
    node->filename = filename;
    node->line = line;
    /* update stats */
    anv_leaks__settings.info.total_allocated += size;
    ++anv_leaks__settings.info.malloc_count;
    anv_leaks__map_add(&anv_leaks__settings.map, node);
    /* report allocation to output */
    if (is_realloc)
        fprintf(anv_leaks__settings.output, "[%s:%d] <%p> <realloc> malloc(%d)\n",
            filename, line, mem, size);
    else
        fprintf(anv_leaks__settings.output, "[%s:%d] <%p> malloc(%d)\n",
            filename, line, mem, size);
    return mem;
}

void
anv_leaks_free_(void *mem, const char *filename, int line)
{
    anv_leaks__alloc_map *i;

    ++anv_leaks__settings.info.free_count;
    anv_leaks__map_foreach(&anv_leaks__settings.map, i) {
        if (i->block == mem) {
            anv_leaks__settings.info.total_freed += i->size;
            fprintf(anv_leaks__settings.output, "[%s:%d] <%p> free(%d)\n",
                filename, line, i->block, i->size);
            break;
        }
    }
    if (i != &anv_leaks__settings.map) {
        anv_leaks__map_delete(i);
        free(i);
        free(mem);
        return;
    }
    anv_leaks__assert(!mem && "attempt to free a NULL pointer.");
    anv_leaks__assert(0 && "attempt to free an unkwnown memory block.");
}

void *
anv_leaks_calloc_(size_t num, size_t size, const char *filename, int line)
{
    void *mem;
    anv_leaks__alloc_map *node;

    anv_leaks__assert(size != 0);

    /* alloc new node */
    node = malloc(sizeof(anv_leaks__alloc_map));
    anv_leaks__assert(node);
    mem = calloc(num, size);
    anv_leaks__assert(mem);
    /* intialize new node */
    node->block = mem;
    node->size = num * size;
    node->filename = filename;
    node->line = line;
    /* update stats */
    anv_leaks__settings.info.total_allocated += num * size;
    ++anv_leaks__settings.info.calloc_count;
    anv_leaks__map_add(&anv_leaks__settings.map, node);
    /* report allocation to output */
    fprintf(anv_leaks__settings.output, "[%s:%d] <%p> calloc(%d, %d) | total: %d\n",
        filename, line, mem, num, size, num * size);
    return mem;
}

void *
anv_leaks_realloc_(void *mem, size_t size, const char *filename, int line)
{
    void *new_mem;
    anv_leaks__alloc_map *node;
    size_t old_size = 0;
    int found = 0;

    anv_leaks__assert(size != 0);

    if (!mem)
        return anv_leaks_malloc_(size, filename, line, 1);
    new_mem = realloc(mem, size);
    anv_leaks__assert(new_mem);
    /* update memory */
    anv_leaks__map_foreach(&anv_leaks__settings.map, node) {
        if (node->block == mem) {
            node->block = new_mem;
            old_size = node->size;
            node->size = size;
            found = 1;
            break;
        }
    }
    anv_leaks__assert(!found && "reallocated unknown memory block.");
    /* update stats */
    anv_leaks__settings.info.total_allocated += (size - old_size);
    ++anv_leaks__settings.info.realloc_count;
    /* report reallocation to output */
    fprintf(anv_leaks__settings.output, "[%s:%d] <%p> realloc(from: %d, to: %d) | diff: %d\n",
        filename, line, new_mem, old_size, size, size - old_size);
    return new_mem;
}

#endif

#endif /* ANV_LEAKS_H */
