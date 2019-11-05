/*
 *
 *      Filename: gbw_mpool.h
 *
 *        Author: shajf,csp001314@163.com
 *   Description: ---
 *        Create: 2017-01-09 10:07:10
 * Last Modified: 2017-01-09 10:07:10
 */

#ifndef GBW_MPOOL_H
#define GBW_MPOOL_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <malloc.h>

#define gbw_unused __attribute__((__unused__))

#ifndef GBW_ALIGNMENT
#define GBW_ALIGNMENT sizeof(unsigned long)
#endif

#define gbw_align(d,a) (((d) + (a - 1)) & ~(a - 1))

#define gbw_align_ptr(p, a) \
	(void *) (((uintptr_t) (p) + ((uintptr_t) a - 1)) & ~((uintptr_t) a - 1))   

/**
 * short definition to mark a function parameter unused
 */
#define gbw_unused __attribute__((__unused__))

/**
 * definition to mark a variable or function parameter as used so
 * as to avoid a compiler warning
 */
#define GBW_SET_USED(x) (void)(x)

/*********** Macros for pointer arithmetic ********/

/**
 * add a byte-value offset from a pointer
 */
#define GBW_PTR_ADD(ptr, x) ((void*)((uintptr_t)(ptr) + (x)))

/**
 * subtract a byte-value offset from a pointer
 */
#define GBW_PTR_SUB(ptr, x) ((void*)((uintptr_t)ptr - (x)))

/**
 * get the difference between two pointer values, i.e. how far apart
 * in bytes are the locations they point two. It is assumed that
 * ptr1 is greater than ptr2.
 */
#define GBW_PTR_DIFF(ptr1, ptr2) ((uintptr_t)(ptr1) - (uintptr_t)(ptr2))

/*********** Macros/static functions for doing alignment ********/


/**
 * Macro to align a pointer to a given power-of-two. The resultant
 * pointer will be a pointer of the same type as the first parameter, and
 * point to an address no higher than the first parameter. Second parameter
 * must be a power-of-two value.
 */
#define GBW_PTR_ALIGN_FLOOR(ptr, align) \
	((typeof(ptr))GBW_ALIGN_FLOOR((uintptr_t)ptr, align))

/**
 * Macro to align a value to a given power-of-two. The resultant value
 * will be of the same type as the first parameter, and will be no
 * bigger than the first parameter. Second parameter must be a
 * power-of-two value.
 */
#define GBW_ALIGN_FLOOR(val, align) \
	(typeof(val))((val) & (~((typeof(val))((align) - 1))))

/**
 * Macro to align a pointer to a given power-of-two. The resultant
 * pointer will be a pointer of the same type as the first parameter, and
 * point to an address no lower than the first parameter. Second parameter
 * must be a power-of-two value.
 */
#define GBW_PTR_ALIGN_CEIL(ptr, align) \
	GBW_PTR_ALIGN_FLOOR((typeof(ptr))GBW_PTR_ADD(ptr, (align) - 1), align)

/**
 * Macro to align a value to a given power-of-two. The resultant value
 * will be of the same type as the first parameter, and will be no lower
 * than the first parameter. Second parameter must be a power-of-two
 * value.
 */
#define GBW_ALIGN_CEIL(val, align) \
	GBW_ALIGN_FLOOR(((val) + ((typeof(val)) (align) - 1)), align)

/**
 * Macro to align a pointer to a given power-of-two. The resultant
 * pointer will be a pointer of the same type as the first parameter, and
 * point to an address no lower than the first parameter. Second parameter
 * must be a power-of-two value.
 * This function is the same as GBW_PTR_ALIGN_CEIL
 */
#define GBW_PTR_ALIGN(ptr, align) GBW_PTR_ALIGN_CEIL(ptr, align)

/**
 * Macro to align a value to a given power-of-two. The resultant
 * value will be of the same type as the first parameter, and
 * will be no lower than the first parameter. Second parameter
 * must be a power-of-two value.
 * This function is the same as GBW_ALIGN_CEIL
 */
#define GBW_ALIGN(val, align) GBW_ALIGN_CEIL(val, align)

#define gbw_align_up(d,a) (((d)+(a-1))&~(a-1))

/**
 * Checks if a pointer is aligned to a given power-of-two value
 *
 * @param ptr
 *   The pointer whose alignment is to be checked
 * @param align
 *   The power-of-two value to which the ptr should be aligned
 *
 * @return
 *   True(1) where the pointer is correctly aligned, false(0) otherwise
 */
static inline int
gbw_is_aligned(void *ptr, unsigned align)
{
	return GBW_PTR_ALIGN(ptr, align) == ptr;
}
typedef void (*gbw_pool_cleanup_pt)(void *data);
typedef struct gbw_pool_cleanup_t  gbw_pool_cleanup_t;
typedef struct gbw_pool_large_t gbw_pool_large_t;
typedef struct gbw_pool_t gbw_pool_t;
typedef struct gbw_pool_data_t gbw_pool_data_t;

#define GBW_MAX_ALLOC_FROM_POOL  (4096 - 1)

#define GBW_DEFAULT_POOL_SIZE    (16 * 1024)

#define GBW_POOL_ALIGNMENT       16
#define GBW_MIN_POOL_SIZE                                                     \
    gbw_align((sizeof(gbw_pool_t) + 2 * sizeof(gbw_pool_large_t)),            \
              GBW_POOL_ALIGNMENT)



struct gbw_pool_cleanup_t {
    gbw_pool_cleanup_pt   handler;
    void                 *data;
    gbw_pool_cleanup_t   *next;
};


struct gbw_pool_large_t {
    gbw_pool_large_t     *next;
    void                 *alloc;
};


struct gbw_pool_data_t {
    void               *last;
    void               *end;
    gbw_pool_t           *next;
    size_t           failed;
};


struct gbw_pool_t {

    gbw_pool_data_t       d;
    size_t                max;
    gbw_pool_t           *current;
    gbw_pool_large_t     *large;
    gbw_pool_cleanup_t   *cleanup;
};

extern gbw_pool_t *
gbw_pool_create(size_t size);

extern void 
gbw_pool_destroy(gbw_pool_t *pool);

extern void 
gbw_pool_reset(gbw_pool_t *pool);

extern void *
gbw_palloc(gbw_pool_t *pool, size_t size);

extern void *
gbw_pnalloc(gbw_pool_t *pool, size_t size);

extern void *
gbw_pcalloc(gbw_pool_t *pool, size_t size);

extern void *
gbw_pmemalign(gbw_pool_t *pool, size_t size, size_t alignment);

extern int 
gbw_pfree(gbw_pool_t *pool, void *p);


extern gbw_pool_cleanup_t *
gbw_pool_cleanup_add(gbw_pool_t *p, size_t size);

#endif /* GBW_MPOOL_H */
