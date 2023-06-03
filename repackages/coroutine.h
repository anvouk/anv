/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2013 codingnow.com Inc.
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
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
/*
 * Repackaged as a single header C lib and cleaned up by Andrea Vouk.
 * https://github.com/anvouk/anv
 *
 * Changes:
 * - Use C99 enums where appropriate.
 * - Prefix all stuff.
 * - typedef all structs.
 * - Remove unused internal code.
 * - Check memory allocation for error.
 */

#ifndef C_COROUTINE_H
#define C_COROUTINE_H

typedef enum co_status_t {
    COROUTINE_DEAD = 0,
    COROUTINE_READY = 1,
    COROUTINE_RUNNING = 2,
    COROUTINE_SUSPEND = 3
} co_status_t;

typedef struct co_schedule_t co_schedule_t;

typedef void (*coroutine_func_fn)(struct co_schedule_t *, void *ud);

struct co_schedule_t *coroutine_open(void);
void coroutine_close(struct co_schedule_t *S);

int coroutine_new(struct co_schedule_t *, coroutine_func_fn, void *ud);
void coroutine_resume(struct co_schedule_t *, int id);
co_status_t coroutine_status(struct co_schedule_t *, int id);
int coroutine_running(struct co_schedule_t *);
void coroutine_yield(struct co_schedule_t *);

#ifdef COROUTINE_IMPLEMENTATION

#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#if __APPLE__ && __MACH__
#include <sys/ucontext.h>
#else
#include <ucontext.h>
#endif

#ifndef CO_STACK_SIZE
#define CO_STACK_SIZE (1024 * 1024)
#endif

#ifndef CO_DEFAULT_COROUTINE
#define CO_DEFAULT_COROUTINE 16
#endif

typedef struct co_coroutine_t co_coroutine_t;

typedef struct co_schedule_t {
    char stack[CO_STACK_SIZE];
    ucontext_t main;
    int nco;
    int cap;
    int running;
    co_coroutine_t **co;
} co_schedule_t;

typedef struct co_coroutine_t {
    coroutine_func_fn func;
    void *ud;
    ucontext_t ctx;
    ptrdiff_t cap;
    ptrdiff_t size;
    co_status_t status;
    char *stack;
} co_coroutine_t;

static co_coroutine_t *
co__new(coroutine_func_fn func, void *ud)
{
    co_coroutine_t *co = malloc(sizeof(*co));
    if (!co) {
        return NULL;
    }
    co->func = func;
    co->ud = ud;
    co->cap = 0;
    co->size = 0;
    co->status = COROUTINE_READY;
    co->stack = NULL;
    return co;
}

static void
co__delete(co_coroutine_t *co)
{
    free(co->stack);
    free(co);
}

co_schedule_t *
coroutine_open(void)
{
    co_schedule_t *S = malloc(sizeof(*S));
    if (!S) {
        return NULL;
    }

    S->nco = 0;
    S->cap = CO_DEFAULT_COROUTINE;
    S->running = -1;
    S->co = malloc(sizeof(co_coroutine_t *) * S->cap);
    if (!S->co) {
        free(S);
        return NULL;
    }
    memset(S->co, 0, sizeof(co_coroutine_t *) * S->cap);
    return S;
}

void
coroutine_close(co_schedule_t *S)
{
    int i;
    for (i = 0; i < S->cap; i++) {
        co_coroutine_t *co = S->co[i];
        if (co) {
            co__delete(co);
        }
    }
    free(S->co);
    S->co = NULL;
    free(S);
}

int
coroutine_new(co_schedule_t *S, coroutine_func_fn func, void *ud)
{
    co_coroutine_t *co = co__new(func, ud);
    if (S->nco >= S->cap) {
        int id = S->cap;
        void *mem = realloc(S->co, S->cap * 2 * sizeof(co_coroutine_t *));
        if (!mem) {
            return -1;
        }
        S->co = mem;
        memset(S->co + S->cap, 0, sizeof(co_coroutine_t *) * S->cap);
        S->co[S->cap] = co;
        S->cap *= 2;
        ++S->nco;
        return id;
    } else {
        int i;
        for (i = 0; i < S->cap; i++) {
            int id = (i + S->nco) % S->cap;
            if (S->co[id] == NULL) {
                S->co[id] = co;
                ++S->nco;
                return id;
            }
        }
    }
    assert(0);
    return -1;
}

static void
co__mainfunc(uint32_t low32, uint32_t hi32)
{
    uintptr_t ptr = (uintptr_t)low32 | ((uintptr_t)hi32 << 32);
    co_schedule_t *S = (co_schedule_t *)ptr;
    int id = S->running;
    co_coroutine_t *C = S->co[id];
    C->func(S, C->ud);
    co__delete(C);
    S->co[id] = NULL;
    --S->nco;
    S->running = -1;
}

void
coroutine_resume(co_schedule_t *S, int id)
{
    assert(S->running == -1);
    assert(id >= 0 && id < S->cap);
    co_coroutine_t *C = S->co[id];
    if (C == NULL) {
        return;
    }
    co_status_t status = C->status;
    switch (status) {
        case COROUTINE_READY:
            getcontext(&C->ctx);
            C->ctx.uc_stack.ss_sp = S->stack;
            C->ctx.uc_stack.ss_size = CO_STACK_SIZE;
            C->ctx.uc_link = &S->main;
            S->running = id;
            C->status = COROUTINE_RUNNING;
            uintptr_t ptr = (uintptr_t)S;
            makecontext(
                &C->ctx,
                (void (*)(void))co__mainfunc,
                2,
                (uint32_t)ptr,
                (uint32_t)(ptr >> 32)
            );
            swapcontext(&S->main, &C->ctx);
            break;
        case COROUTINE_SUSPEND:
            memcpy(S->stack + CO_STACK_SIZE - C->size, C->stack, C->size);
            S->running = id;
            C->status = COROUTINE_RUNNING;
            swapcontext(&S->main, &C->ctx);
            break;
        default:
            assert(0);
    }
}

static void
co__save_stack(co_coroutine_t *C, char *top)
{
    char dummy = 0;
    assert(top - &dummy <= CO_STACK_SIZE);
    if (C->cap < top - &dummy) {
        free(C->stack);
        C->cap = top - &dummy;
        // FIXME: handle OOM
        C->stack = malloc(C->cap);
        assert(C->stack);
    }
    C->size = top - &dummy;
    memcpy(C->stack, &dummy, C->size);
}

void
coroutine_yield(co_schedule_t *S)
{
    int id = S->running;
    assert(id >= 0);
    co_coroutine_t *C = S->co[id];
    assert((char *)&C > S->stack);
    co__save_stack(C, S->stack + CO_STACK_SIZE);
    C->status = COROUTINE_SUSPEND;
    S->running = -1;
    swapcontext(&C->ctx, &S->main);
}

co_status_t
coroutine_status(co_schedule_t *S, int id)
{
    assert(id >= 0 && id < S->cap);
    if (S->co[id] == NULL) {
        return COROUTINE_DEAD;
    }
    return S->co[id]->status;
}

int
coroutine_running(co_schedule_t *S)
{
    return S->running;
}

#endif /* COROUTINE_IMPLEMENTATION */

#endif /* C_COROUTINE_H */
