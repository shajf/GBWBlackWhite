/*
 *
 *      Filename: gbw_mpool.c
 *
 *        Author: shajf,csp001314@163.com
 *   Description: ---
 *        Create: 2017-01-09 11:04:58
 * Last Modified: 2018-05-04 12:08:59
 */

#include "gbw_mpool.h"

gbw_pool_t *
gbw_pool_create(size_t size)
{
    gbw_pool_t  *p;

    p = (gbw_pool_t*)memalign(GBW_POOL_ALIGNMENT, size);

    if (p == NULL) {
        return NULL;
    }

    p->d.last = (void *) p + sizeof(gbw_pool_t);
    p->d.end = (void *) p + size;
    p->d.next = NULL;
    p->d.failed = 0;

    size = size - sizeof(gbw_pool_t);
    p->max = (size < GBW_MAX_ALLOC_FROM_POOL) ? size : GBW_MAX_ALLOC_FROM_POOL;

    p->current = p;
    p->large = NULL;
    p->cleanup = NULL;

    return p;
}

void
gbw_pool_destroy(gbw_pool_t *pool)
{
    gbw_pool_t          *p, *n;
    gbw_pool_large_t    *l;
    gbw_pool_cleanup_t  *c;

    for (c = pool->cleanup; c; c = c->next) {
        if (c->handler) {
            c->handler(c->data);
        }
    }

    for (l = pool->large; l; l = l->next) {
        if (l->alloc) {
            free(l->alloc);
        }
    }

    for (p = pool, n = pool->d.next; /* void */; p = n, n = n->d.next) {
        free(p);

        if (n == NULL) {
            break;
        }
    }
}


void
gbw_pool_reset(gbw_pool_t *pool)
{
    gbw_pool_t        *p;
    gbw_pool_large_t  *l;

    for (l = pool->large; l; l = l->next) {
        if (l->alloc) {
            free(l->alloc);
        }
    }

    for (p = pool; p; p = p->d.next) {
        p->d.last = (void *) p + sizeof(gbw_pool_t);
        p->d.failed = 0;
    }

    pool->current = pool;
    pool->large = NULL;
}

static void *
gbw_palloc_block(gbw_pool_t *pool, size_t size)
{
    void      *m;
    size_t       psize;
    gbw_pool_t  *p, *new;

    psize = (size_t) (pool->d.end - (void *) pool);

    m = memalign(GBW_POOL_ALIGNMENT, psize);

    if (m == NULL) {
        return NULL;
    }

    new = (gbw_pool_t *) m;

    new->d.end = m + psize;
    new->d.next = NULL;
    new->d.failed = 0;

    m += sizeof(gbw_pool_data_t);
    m = gbw_align_ptr(m, GBW_ALIGNMENT);
    new->d.last = m + size;

    for (p = pool->current; p->d.next; p = p->d.next) {
        if (p->d.failed++ > 4) {
            pool->current = p->d.next;
        }
    }

    p->d.next = new;

    return m;
}

static inline void *
gbw_palloc_small(gbw_pool_t *pool, size_t size, unsigned int align)
{
    void      *m;
    gbw_pool_t  *p;

    p = pool->current;

    do {
        m = p->d.last;

        if (align) {
            m = gbw_align_ptr(m, GBW_ALIGNMENT);
        }

        if ((size_t) (p->d.end - m) >= size) {
            p->d.last = m + size;

            return m;
        }

        p = p->d.next;

    } while (p);

    return gbw_palloc_block(pool, size);
}




static void *
gbw_palloc_large(gbw_pool_t *pool, size_t size)
{
    void              *p;
    unsigned int         n;
    gbw_pool_large_t  *large;

    p = malloc(size);
    if (p == NULL) {
        return NULL;
    }

    n = 0;

    for (large = pool->large; large; large = large->next) {
        if (large->alloc == NULL) {
            large->alloc = p;
            return p;
        }

        if (n++ > 3) {
            break;
        }
    }

    large = gbw_palloc_small(pool, sizeof(gbw_pool_large_t), 1);
    if (large == NULL) {
        free(p);
        return NULL;
    }

    large->alloc = p;
    large->next = pool->large;
    pool->large = large;

    return p;
}

void *
gbw_palloc(gbw_pool_t *pool, size_t size)
{
    if (size <= pool->max) {
        return gbw_palloc_small(pool, size, 1);
    }

    return gbw_palloc_large(pool, size);
}


void *
gbw_pnalloc(gbw_pool_t *pool, size_t size)
{
    if (size <= pool->max) {
        return gbw_palloc_small(pool, size, 0);
    }

    return gbw_palloc_large(pool, size);
}

void *
gbw_pmemalign(gbw_pool_t *pool, size_t size, size_t alignment)
{
    void              *p;
    gbw_pool_large_t  *large;

    p = memalign(alignment, size);
    if (p == NULL) {
        return NULL;
    }

    large = gbw_palloc_small(pool, sizeof(gbw_pool_large_t), 1);
    if (large == NULL) {
        free(p);
        return NULL;
    }

    large->alloc = p;
    large->next = pool->large;
    pool->large = large;

    return p;
}


int
gbw_pfree(gbw_pool_t *pool, void *p)
{
    gbw_pool_large_t  *l;

    for (l = pool->large; l; l = l->next) {
        if (p == l->alloc) {
            free(l->alloc);
            l->alloc = NULL;

            return 0;
        }
    }

    return -1;
}


void *
gbw_pcalloc(gbw_pool_t *pool, size_t size)
{
    void *p;

    p = gbw_palloc(pool, size);
    if (p) {
        memset(p, 0,size);
    }

    return p;
}


gbw_pool_cleanup_t *
gbw_pool_cleanup_add(gbw_pool_t *p, size_t size)
{
    gbw_pool_cleanup_t  *c;

    c = gbw_palloc(p, sizeof(gbw_pool_cleanup_t));
    if (c == NULL) {
        return NULL;
    }

    if (size) {
        c->data = gbw_palloc(p, size);
        if (c->data == NULL) {
            return NULL;
        }

    } else {
        c->data = NULL;
    }

    c->handler = NULL;
    c->next = p->cleanup;

    p->cleanup = c;

    return c;
}

