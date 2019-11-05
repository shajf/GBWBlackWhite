/*
 *
 *      Filename: gbw_tables.c
 *
 *        Author: shajf,csp001314@163.com
 *   Description: ---
 *        Create: 2017-01-10 14:05:22
 * Last Modified: 2017-01-10 14:20:53
 */

#include "gbw_tables.h"
#include "gbw_string.h"
#include <stdarg.h>

/*****************************************************************
 * This file contains array and gbw_table_t functions only.
 */

/*****************************************************************
 *
 * The 'array' functions...
 */

static void make_array_core(gbw_array_header_t *res, gbw_pool_t *p,
			    int nelts, int elt_size, int clear)
{
    /*
     * Assure sanity if someone asks for
     * array of zero elts.
     */
    if (nelts < 1) {
        nelts = 1;
    }

    if (clear) {
        res->elts = gbw_pcalloc(p, nelts * elt_size);
    }
    else {
        res->elts = gbw_palloc(p, nelts * elt_size);
    }

    res->pool = p;
    res->elt_size = elt_size;
    res->nelts = 0;		/* No active elements yet... */
    res->nalloc = nelts;	/* ...but this many allocated */
}

int gbw_is_empty_array(const gbw_array_header_t *a)
{
    return ((a == NULL) || (a->nelts == 0));
}

gbw_array_header_t * gbw_array_make(gbw_pool_t *p,
						int nelts, int elt_size)
{
    gbw_array_header_t *res;

    res = (gbw_array_header_t *) gbw_palloc(p, sizeof(gbw_array_header_t));
    make_array_core(res, p, nelts, elt_size, 1);
    return res;
}

void gbw_array_clear(gbw_array_header_t *arr)
{
    arr->nelts = 0;
}

void * gbw_array_pop(gbw_array_header_t *arr)
{
    if (gbw_is_empty_array(arr)) {
        return NULL;
    }
   
    return arr->elts + (arr->elt_size * (--arr->nelts));
}

void * gbw_array_push(gbw_array_header_t *arr)
{
    if (arr->nelts == arr->nalloc) {
        int new_size = (arr->nalloc <= 0) ? 1 : arr->nalloc * 2;
        char *new_data;

        new_data = gbw_palloc(arr->pool, arr->elt_size * new_size);

        memcpy(new_data, arr->elts, arr->nalloc * arr->elt_size);
        memset(new_data + arr->nalloc * arr->elt_size, 0,
               arr->elt_size * (new_size - arr->nalloc));
        arr->elts = new_data;
        arr->nalloc = new_size;
    }

    ++arr->nelts;
    return arr->elts + (arr->elt_size * (arr->nelts - 1));
}

static void *gbw_array_push_noclear(gbw_array_header_t *arr)
{
    if (arr->nelts == arr->nalloc) {
        int new_size = (arr->nalloc <= 0) ? 1 : arr->nalloc * 2;
        char *new_data;

        new_data = gbw_palloc(arr->pool, arr->elt_size * new_size);

        memcpy(new_data, arr->elts, arr->nalloc * arr->elt_size);
        arr->elts = new_data;
        arr->nalloc = new_size;
    }

    ++arr->nelts;
    return arr->elts + (arr->elt_size * (arr->nelts - 1));
}

void gbw_array_cat(gbw_array_header_t *dst,
			       const gbw_array_header_t *src)
{
    int elt_size = dst->elt_size;

    if (dst->nelts + src->nelts > dst->nalloc) {
	int new_size = (dst->nalloc <= 0) ? 1 : dst->nalloc * 2;
	char *new_data;

	while (dst->nelts + src->nelts > new_size) {
	    new_size *= 2;
	}

	new_data = gbw_pcalloc(dst->pool, elt_size * new_size);
	memcpy(new_data, dst->elts, dst->nalloc * elt_size);

	dst->elts = new_data;
	dst->nalloc = new_size;
    }

    memcpy(dst->elts + dst->nelts * elt_size, src->elts,
	   elt_size * src->nelts);
    dst->nelts += src->nelts;
}

gbw_array_header_t * gbw_array_copy(gbw_pool_t *p,
						const gbw_array_header_t *arr)
{
    gbw_array_header_t *res =
        (gbw_array_header_t *) gbw_palloc(p, sizeof(gbw_array_header_t));
    make_array_core(res, p, arr->nalloc, arr->elt_size, 0);

    memcpy(res->elts, arr->elts, arr->elt_size * arr->nelts);
    res->nelts = arr->nelts;
    memset(res->elts + res->elt_size * res->nelts, 0,
           res->elt_size * (res->nalloc - res->nelts));
    return res;
}

/* This cute function copies the array header *only*, but arranges
 * for the data section to be copied on the first push or arraycat.
 * It's useful when the elements of the array being copied are
 * read only, but new stuff *might* get added on the end; we have the
 * overhead of the full copy only where it is really needed.
 */

static inline void copy_array_hdr_core(gbw_array_header_t *res,
					   const gbw_array_header_t *arr)
{
    res->elts = arr->elts;
    res->elt_size = arr->elt_size;
    res->nelts = arr->nelts;
    res->nalloc = arr->nelts;	/* Force overflow on push */
}

gbw_array_header_t *
    gbw_array_copy_hdr(gbw_pool_t *p,
		       const gbw_array_header_t *arr)
{
    gbw_array_header_t *res;

    res = (gbw_array_header_t *) gbw_palloc(p, sizeof(gbw_array_header_t));
    res->pool = p;
    copy_array_hdr_core(res, arr);
    return res;
}

/* The above is used here to avoid consing multiple new array bodies... */

gbw_array_header_t *
    gbw_array_append(gbw_pool_t *p,
		      const gbw_array_header_t *first,
		      const gbw_array_header_t *second)
{
    gbw_array_header_t *res = gbw_array_copy_hdr(p, first);

    gbw_array_cat(res, second);
    return res;
}

/* gbw_array_pstrcat generates a new string from the gbw_pool_t containing
 * the concatenated sequence of substrings referenced as elements within
 * the array.  The string will be empty if all substrings are empty or null,
 * or if there are no elements in the array.
 * If sep is non-NUL, it will be inserted between elements as a separator.
 */
char * gbw_array_pstrcat(gbw_pool_t *p,
				     const gbw_array_header_t *arr,
				     const char sep)
{
    char *cp, *res, **strpp;
    size_t len;
    int i;

    if (arr->nelts <= 0 || arr->elts == NULL) {    /* Empty table? */
        return (char *) gbw_pcalloc(p, 1);
    }

    /* Pass one --- find length of required string */

    len = 0;
    for (i = 0, strpp = (char **) arr->elts; ; ++strpp) {
        if (strpp && *strpp != NULL) {
            len += strlen(*strpp);
        }
        if (++i >= arr->nelts) {
            break;
	}
        if (sep) {
            ++len;
	}
    }

    /* Allocate the required string */

    res = (char *) gbw_palloc(p, len + 1);
    cp = res;

    /* Pass two --- copy the argument strings into the result space */

    for (i = 0, strpp = (char **) arr->elts; ; ++strpp) {
        if (strpp && *strpp != NULL) {
            len = strlen(*strpp);
            memcpy(cp, *strpp, len);
            cp += len;
        }
        if (++i >= arr->nelts) {
            break;
	}
        if (sep) {
            *cp++ = sep;
	}
    }

    *cp = '\0';

    /* Return the result string */

    return res;
}


/*****************************************************************
 *
 * The "table" functions.
 */

#if GBW_CHARSET_EBCDIC
#define CASE_MASK 0xbfbfbfbf
#else
#define CASE_MASK 0xdfdfdfdf
#endif

#define TABLE_HASH_SIZE 32
#define TABLE_INDEX_MASK 0x1f
#define TABLE_HASH(key)  (TABLE_INDEX_MASK & *(unsigned char *)(key))
#define TABLE_INDEX_IS_INITIALIZED(t, i) ((t)->index_initialized & (1 << (i)))
#define TABLE_SET_INDEX_INITIALIZED(t, i) ((t)->index_initialized |= (1 << (i)))

/* Compute the "checksum" for a key, consisting of the first
 * 4 bytes, normalized for case-insensitivity and packed into
 * an int...this checksum allows us to do a single integer
 * comparison as a fast check to determine whether we can
 * skip a strcasecmp
 */
#define COMPUTE_KEY_CHECKSUM(key, checksum)    \
{                                              \
    const char *k = (key);                     \
    uint32_t c = (uint32_t)*k;         \
    (checksum) = c;                            \
    (checksum) <<= 8;                          \
    if (c) {                                   \
        c = (uint32_t)*++k;                \
        checksum |= c;                         \
    }                                          \
    (checksum) <<= 8;                          \
    if (c) {                                   \
        c = (uint32_t)*++k;                \
        checksum |= c;                         \
    }                                          \
    (checksum) <<= 8;                          \
    if (c) {                                   \
        c = (uint32_t)*++k;                \
        checksum |= c;                         \
    }                                          \
    checksum &= CASE_MASK;                     \
}

/** The opaque string-content table type */
struct gbw_table_t {
    /* This has to be first to promote backwards compatibility with
     * older modules which cast a gbw_table_t * to an gbw_array_header_t *...
     * they should use the gbw_table_elts() function for most of the
     * cases they do this for.
     */
    /** The underlying array for the table */
    gbw_array_header_t a;
#ifdef MAKE_TABLE_PROFILE
    /** Who created the array. */
    void *creator;
#endif
    /* An index to speed up table lookups.  The way this works is:
     *   - Hash the key into the index:
     *     - index_first[TABLE_HASH(key)] is the offset within
     *       the table of the first entry with that key
     *     - index_last[TABLE_HASH(key)] is the offset within
     *       the table of the last entry with that key
     *   - If (and only if) there is no entry in the table whose
     *     key hashes to index element i, then the i'th bit
     *     of index_initialized will be zero.  (Check this before
     *     trying to use index_first[i] or index_last[i]!)
     */
    uint32_t index_initialized;
    int index_first[TABLE_HASH_SIZE];
    int index_last[TABLE_HASH_SIZE];
};

/* keep state for gbw_table_getm() */
typedef struct
{
    gbw_pool_t *p;
    const char *first;
    gbw_array_header_t *merged;
} table_getm_t;

/*
 * NOTICE: if you tweak this you should look at is_empty_table() 
 * and table_elts() in alloc.h
 */
#ifdef MAKE_TABLE_PROFILE
static gbw_table_entry_t *do_table_push(const char *func, gbw_table_t *t)
{
    if (t->a.nelts == t->a.nalloc) {
        fprintf(stderr, "%s: table created by %p hit limit of %u\n",
                func ? func : "table_push", t->creator, t->a.nalloc);
    }
    return (gbw_table_entry_t *) gbw_array_push_noclear(&t->a);
}
#if defined(__GNUC__) && __GNUC__ >= 2
#define table_push(t) do_table_push(__FUNCTION__, t)
#else
#define table_push(t) do_table_push(NULL, t)
#endif
#else /* MAKE_TABLE_PROFILE */
#define table_push(t)	((gbw_table_entry_t *) gbw_array_push_noclear(&(t)->a))
#endif /* MAKE_TABLE_PROFILE */

const gbw_array_header_t * gbw_table_elts(const gbw_table_t *t)
{
    return (const gbw_array_header_t *)t;
}

int gbw_is_empty_table(const gbw_table_t *t)
{
    return ((t == NULL) || (t->a.nelts == 0));
}

gbw_table_t * gbw_table_make(gbw_pool_t *p, int nelts)
{
    gbw_table_t *t = gbw_palloc(p, sizeof(gbw_table_t));

    make_array_core(&t->a, p, nelts, sizeof(gbw_table_entry_t), 0);
#ifdef MAKE_TABLE_PROFILE
    t->creator = __builtin_return_address(0);
#endif
    t->index_initialized = 0;
    return t;
}

gbw_table_t * gbw_table_copy(gbw_pool_t *p, const gbw_table_t *t)
{
    gbw_table_t *new = gbw_palloc(p, sizeof(gbw_table_t));


    make_array_core(&new->a, p, t->a.nalloc, sizeof(gbw_table_entry_t), 0);
    memcpy(new->a.elts, t->a.elts, t->a.nelts * sizeof(gbw_table_entry_t));
    new->a.nelts = t->a.nelts;
    memcpy(new->index_first, t->index_first, sizeof(int) * TABLE_HASH_SIZE);
    memcpy(new->index_last, t->index_last, sizeof(int) * TABLE_HASH_SIZE);
    new->index_initialized = t->index_initialized;
    return new;
}

gbw_table_t * gbw_table_clone(gbw_pool_t *p, const gbw_table_t *t)
{
    const gbw_array_header_t *array = gbw_table_elts(t);
    gbw_table_entry_t *elts = (gbw_table_entry_t *) array->elts;
    gbw_table_t *new = gbw_table_make(p, array->nelts);
    int i;

    for (i = 0; i < array->nelts; i++) {
        gbw_table_add(new, elts[i].key, elts[i].val);
    }

    return new;
}

static void table_reindex(gbw_table_t *t)
{
    int i;
    int hash;
    gbw_table_entry_t *next_elt = (gbw_table_entry_t *) t->a.elts;

    t->index_initialized = 0;
    for (i = 0; i < t->a.nelts; i++, next_elt++) {
        hash = TABLE_HASH(next_elt->key);
        t->index_last[hash] = i;
        if (!TABLE_INDEX_IS_INITIALIZED(t, hash)) {
            t->index_first[hash] = i;
            TABLE_SET_INDEX_INITIALIZED(t, hash);
        }
    }
}

void gbw_table_clear(gbw_table_t *t)
{
    t->a.nelts = 0;
    t->index_initialized = 0;
}

const char * gbw_table_get(const gbw_table_t *t, const char *key)
{
    gbw_table_entry_t *next_elt;
    gbw_table_entry_t *end_elt;
    uint32_t checksum;
    int hash;

    if (key == NULL) {
	return NULL;
    }

    hash = TABLE_HASH(key);
    if (!TABLE_INDEX_IS_INITIALIZED(t, hash)) {
        return NULL;
    }
    COMPUTE_KEY_CHECKSUM(key, checksum);
    next_elt = ((gbw_table_entry_t *) t->a.elts) + t->index_first[hash];;
    end_elt = ((gbw_table_entry_t *) t->a.elts) + t->index_last[hash];

    for (; next_elt <= end_elt; next_elt++) {
	if ((checksum == next_elt->key_checksum) &&
            !strcasecmp(next_elt->key, key)) {
	    return next_elt->val;
	}
    }

    return NULL;
}

void gbw_table_set(gbw_table_t *t, const char *key,
                                const char *val)
{
    gbw_table_entry_t *next_elt;
    gbw_table_entry_t *end_elt;
    gbw_table_entry_t *table_end;
    uint32_t checksum;
    int hash;

    COMPUTE_KEY_CHECKSUM(key, checksum);
    hash = TABLE_HASH(key);
    if (!TABLE_INDEX_IS_INITIALIZED(t, hash)) {
        t->index_first[hash] = t->a.nelts;
        TABLE_SET_INDEX_INITIALIZED(t, hash);
        goto add_new_elt;
    }
    next_elt = ((gbw_table_entry_t *) t->a.elts) + t->index_first[hash];;
    end_elt = ((gbw_table_entry_t *) t->a.elts) + t->index_last[hash];
    table_end =((gbw_table_entry_t *) t->a.elts) + t->a.nelts;

    for (; next_elt <= end_elt; next_elt++) {
	if ((checksum == next_elt->key_checksum) &&
            !strcasecmp(next_elt->key, key)) {

            /* Found an existing entry with the same key, so overwrite it */

            int must_reindex = 0;
            gbw_table_entry_t *dst_elt = NULL;

            next_elt->val = gbw_pstrdup(t->a.pool, val);

            /* Remove any other instances of this key */
            for (next_elt++; next_elt <= end_elt; next_elt++) {
                if ((checksum == next_elt->key_checksum) &&
                    !strcasecmp(next_elt->key, key)) {
                    t->a.nelts--;
                    if (!dst_elt) {
                        dst_elt = next_elt;
                    }
                }
                else if (dst_elt) {
                    *dst_elt++ = *next_elt;
                    must_reindex = 1;
                }
            }

            /* If we've removed anything, shift over the remainder
             * of the table (note that the previous loop didn't
             * run to the end of the table, just to the last match
             * for the index)
             */
            if (dst_elt) {
                for (; next_elt < table_end; next_elt++) {
                    *dst_elt++ = *next_elt;
                }
                must_reindex = 1;
            }
            if (must_reindex) {
                table_reindex(t);
            }
            return;
        }
    }

add_new_elt:
    t->index_last[hash] = t->a.nelts;
    next_elt = (gbw_table_entry_t *) table_push(t);
    next_elt->key = gbw_pstrdup(t->a.pool, key);
    next_elt->val = gbw_pstrdup(t->a.pool, val);
    next_elt->key_checksum = checksum;
}

void gbw_table_setn(gbw_table_t *t, const char *key,
                                 const char *val)
{
    gbw_table_entry_t *next_elt;
    gbw_table_entry_t *end_elt;
    gbw_table_entry_t *table_end;
    uint32_t checksum;
    int hash;

    COMPUTE_KEY_CHECKSUM(key, checksum);
    hash = TABLE_HASH(key);
    if (!TABLE_INDEX_IS_INITIALIZED(t, hash)) {
        t->index_first[hash] = t->a.nelts;
        TABLE_SET_INDEX_INITIALIZED(t, hash);
        goto add_new_elt;
    }
    next_elt = ((gbw_table_entry_t *) t->a.elts) + t->index_first[hash];;
    end_elt = ((gbw_table_entry_t *) t->a.elts) + t->index_last[hash];
    table_end =((gbw_table_entry_t *) t->a.elts) + t->a.nelts;

    for (; next_elt <= end_elt; next_elt++) {
	if ((checksum == next_elt->key_checksum) &&
            !strcasecmp(next_elt->key, key)) {

            /* Found an existing entry with the same key, so overwrite it */

            int must_reindex = 0;
            gbw_table_entry_t *dst_elt = NULL;

            next_elt->val = (char *)val;

            /* Remove any other instances of this key */
            for (next_elt++; next_elt <= end_elt; next_elt++) {
                if ((checksum == next_elt->key_checksum) &&
                    !strcasecmp(next_elt->key, key)) {
                    t->a.nelts--;
                    if (!dst_elt) {
                        dst_elt = next_elt;
                    }
                }
                else if (dst_elt) {
                    *dst_elt++ = *next_elt;
                    must_reindex = 1;
                }
            }

            /* If we've removed anything, shift over the remainder
             * of the table (note that the previous loop didn't
             * run to the end of the table, just to the last match
             * for the index)
             */
            if (dst_elt) {
                for (; next_elt < table_end; next_elt++) {
                    *dst_elt++ = *next_elt;
                }
                must_reindex = 1;
            }
            if (must_reindex) {
                table_reindex(t);
            }
            return;
        }
    }

add_new_elt:
    t->index_last[hash] = t->a.nelts;
    next_elt = (gbw_table_entry_t *) table_push(t);
    next_elt->key = (char *)key;
    next_elt->val = (char *)val;
    next_elt->key_checksum = checksum;
}

void gbw_table_unset(gbw_table_t *t, const char *key)
{
    gbw_table_entry_t *next_elt;
    gbw_table_entry_t *end_elt;
    gbw_table_entry_t *dst_elt;
    uint32_t checksum;
    int hash;
    int must_reindex;

    hash = TABLE_HASH(key);
    if (!TABLE_INDEX_IS_INITIALIZED(t, hash)) {
        return;
    }
    COMPUTE_KEY_CHECKSUM(key, checksum);
    next_elt = ((gbw_table_entry_t *) t->a.elts) + t->index_first[hash];
    end_elt = ((gbw_table_entry_t *) t->a.elts) + t->index_last[hash];
    must_reindex = 0;
    for (; next_elt <= end_elt; next_elt++) {
	if ((checksum == next_elt->key_checksum) &&
            !strcasecmp(next_elt->key, key)) {

            /* Found a match: remove this entry, plus any additional
             * matches for the same key that might follow
             */
            gbw_table_entry_t *table_end = ((gbw_table_entry_t *) t->a.elts) +
                t->a.nelts;
            t->a.nelts--;
            dst_elt = next_elt;
            for (next_elt++; next_elt <= end_elt; next_elt++) {
                if ((checksum == next_elt->key_checksum) &&
                    !strcasecmp(next_elt->key, key)) {
                    t->a.nelts--;
                }
                else {
                    *dst_elt++ = *next_elt;
                }
            }

            /* Shift over the remainder of the table (note that
             * the previous loop didn't run to the end of the table,
             * just to the last match for the index)
             */
            for (; next_elt < table_end; next_elt++) {
                *dst_elt++ = *next_elt;
            }
            must_reindex = 1;
            break;
        }
    }
    if (must_reindex) {
        table_reindex(t);
    }
}

void gbw_table_merge(gbw_table_t *t, const char *key,
				 const char *val)
{
    gbw_table_entry_t *next_elt;
    gbw_table_entry_t *end_elt;
    uint32_t checksum;
    int hash;

    COMPUTE_KEY_CHECKSUM(key, checksum);
    hash = TABLE_HASH(key);
    if (!TABLE_INDEX_IS_INITIALIZED(t, hash)) {
        t->index_first[hash] = t->a.nelts;
        TABLE_SET_INDEX_INITIALIZED(t, hash);
        goto add_new_elt;
    }
    next_elt = ((gbw_table_entry_t *) t->a.elts) + t->index_first[hash];
    end_elt = ((gbw_table_entry_t *) t->a.elts) + t->index_last[hash];

    for (; next_elt <= end_elt; next_elt++) {
	if ((checksum == next_elt->key_checksum) &&
            !strcasecmp(next_elt->key, key)) {

            /* Found an existing entry with the same key, so merge with it */
	    next_elt->val = gbw_pstrcat(t->a.pool, next_elt->val, ", ",
                                        val, NULL);
            return;
        }
    }

add_new_elt:
    t->index_last[hash] = t->a.nelts;
    next_elt = (gbw_table_entry_t *) table_push(t);
    next_elt->key = gbw_pstrdup(t->a.pool, key);
    next_elt->val = gbw_pstrdup(t->a.pool, val);
    next_elt->key_checksum = checksum;
}

void gbw_table_mergen(gbw_table_t *t, const char *key,
				  const char *val)
{
    gbw_table_entry_t *next_elt;
    gbw_table_entry_t *end_elt;
    uint32_t checksum;
    int hash;


    COMPUTE_KEY_CHECKSUM(key, checksum);
    hash = TABLE_HASH(key);
    if (!TABLE_INDEX_IS_INITIALIZED(t, hash)) {
        t->index_first[hash] = t->a.nelts;
        TABLE_SET_INDEX_INITIALIZED(t, hash);
        goto add_new_elt;
    }
    next_elt = ((gbw_table_entry_t *) t->a.elts) + t->index_first[hash];;
    end_elt = ((gbw_table_entry_t *) t->a.elts) + t->index_last[hash];

    for (; next_elt <= end_elt; next_elt++) {
	if ((checksum == next_elt->key_checksum) &&
            !strcasecmp(next_elt->key, key)) {

            /* Found an existing entry with the same key, so merge with it */
	    next_elt->val = gbw_pstrcat(t->a.pool, next_elt->val, ", ",
                                        val, NULL);
            return;
        }
    }

add_new_elt:
    t->index_last[hash] = t->a.nelts;
    next_elt = (gbw_table_entry_t *) table_push(t);
    next_elt->key = (char *)key;
    next_elt->val = (char *)val;
    next_elt->key_checksum = checksum;
}

void gbw_table_add(gbw_table_t *t, const char *key,
			       const char *val)
{
    gbw_table_entry_t *elts;
    uint32_t checksum;
    int hash;

    hash = TABLE_HASH(key);
    t->index_last[hash] = t->a.nelts;
    if (!TABLE_INDEX_IS_INITIALIZED(t, hash)) {
        t->index_first[hash] = t->a.nelts;
        TABLE_SET_INDEX_INITIALIZED(t, hash);
    }
    COMPUTE_KEY_CHECKSUM(key, checksum);
    elts = (gbw_table_entry_t *) table_push(t);
    elts->key = gbw_pstrdup(t->a.pool, key);
    elts->val = gbw_pstrdup(t->a.pool, val);
    elts->key_checksum = checksum;
}

void gbw_table_addn(gbw_table_t *t, const char *key,
				const char *val)
{
    gbw_table_entry_t *elts;
    uint32_t checksum;
    int hash;



    hash = TABLE_HASH(key);
    t->index_last[hash] = t->a.nelts;
    if (!TABLE_INDEX_IS_INITIALIZED(t, hash)) {
        t->index_first[hash] = t->a.nelts;
        TABLE_SET_INDEX_INITIALIZED(t, hash);
    }
    COMPUTE_KEY_CHECKSUM(key, checksum);
    elts = (gbw_table_entry_t *) table_push(t);
    elts->key = (char *)key;
    elts->val = (char *)val;
    elts->key_checksum = checksum;
}

gbw_table_t * gbw_table_overlay(gbw_pool_t *p,
					     const gbw_table_t *overlay,
					     const gbw_table_t *base)
{
    gbw_table_t *res;



    res = gbw_palloc(p, sizeof(gbw_table_t));
    /* behave like append_arrays */
    res->a.pool = p;
    copy_array_hdr_core(&res->a, &overlay->a);
    gbw_array_cat(&res->a, &base->a);
    table_reindex(res);
    return res;
}

/* And now for something completely abstract ...

 * For each key value given as a vararg:
 *   run the function pointed to as
 *     int comp(void *r, char *key, char *value);
 *   on each valid key-value pair in the gbw_table_t t that matches the vararg key,
 *   or once for every valid key-value pair if the vararg list is empty,
 *   until the function returns false (0) or we finish the table.
 *
 * Note that we restart the traversal for each vararg, which means that
 * duplicate varargs will result in multiple executions of the function
 * for each matching key.  Note also that if the vararg list is empty,
 * only one traversal will be made and will cut short if comp returns 0.
 *
 * Note that the table_get and table_merge functions assume that each key in
 * the gbw_table_t is unique (i.e., no multiple entries with the same key).  This
 * function does not make that assumption, since it (unfortunately) isn't
 * true for some of Apache's tables.
 *
 * Note that rec is simply passed-on to the comp function, so that the
 * caller can pass additional info for the task.
 *
 * ADDENDUM for gbw_table_vdo():
 * 
 * The caching api will allow a user to walk the header values:
 *
 * gbw_status_t gbw_cache_el_header_walk(gbw_cache_el *el, 
 *    int (*comp)(void *, const char *, const char *), void *rec, ...);
 *
 * So it can be ..., however from there I use a  callback that use a va_list:
 *
 * gbw_status_t (*cache_el_header_walk)(gbw_cache_el *el, 
 *    int (*comp)(void *, const char *, const char *), void *rec, va_list);
 *
 * To pass those ...'s on down to the actual module that will handle walking
 * their headers, in the file case this is actually just an gbw_table - and
 * rather than reimplementing gbw_table_do (which IMHO would be bad) I just
 * called it with the va_list. For mod_shmem_cache I don't need it since I
 * can't use gbw_table's, but mod_file_cache should (though a good hash would
 * be better, but that's a different issue :). 
 *
 * So to make mod_file_cache easier to maintain, it's a good thing
 */
int gbw_table_do(gbw_table_do_callback_fn_t *comp,
                                     void *rec, const gbw_table_t *t, ...)
{
    int rv;

    va_list vp;
    va_start(vp, t);
    rv = gbw_table_vdo(comp, rec, t, vp);
    va_end(vp);

    return rv;
} 

/* XXX: do the semantics of this routine make any sense?  Right now,
 * if the caller passed in a non-empty va_list of keys to search for,
 * the "early termination" facility only terminates on *that* key; other
 * keys will continue to process.  Note that this only has any effect
 * at all if there are multiple entries in the table with the same key,
 * otherwise the called function can never effectively early-terminate
 * this function, as the zero return value is effectively ignored.
 *
 * Note also that this behavior is at odds with the behavior seen if an
 * empty va_list is passed in -- in that case, a zero return value terminates
 * the entire gbw_table_vdo (which is what I think should happen in
 * both cases).
 *
 * If nobody objects soon, I'm going to change the order of the nested
 * loops in this function so that any zero return value from the (*comp)
 * function will cause a full termination of gbw_table_vdo.  I'm hesitant
 * at the moment because these (funky) semantics have been around for a
 * very long time, and although Apache doesn't seem to use them at all,
 * some third-party vendor might.  I can only think of one possible reason
 * the existing semantics would make any sense, and it's very Apache-centric,
 * which is this: if (*comp) is looking for matches of a particular
 * substring in request headers (let's say it's looking for a particular
 * cookie name in the Set-Cookie headers), then maybe it wants to be
 * able to stop searching early as soon as it finds that one and move
 * on to the next key.  That's only an optimization of course, but changing
 * the behavior of this function would mean that any code that tried
 * to do that would stop working right.
 *
 * Sigh.  --JCW, 06/28/02
 */
int gbw_table_vdo(gbw_table_do_callback_fn_t *comp,
                               void *rec, const gbw_table_t *t, va_list vp)
{
    char *argp;
    gbw_table_entry_t *elts = (gbw_table_entry_t *) t->a.elts;
    int vdorv = 1;

    argp = va_arg(vp, char *);
    do {
        int rv = 1, i;
        if (argp) {
            /* Scan for entries that match the next key */
            int hash = TABLE_HASH(argp);
            if (TABLE_INDEX_IS_INITIALIZED(t, hash)) {
                uint32_t checksum;
                COMPUTE_KEY_CHECKSUM(argp, checksum);
                for (i = t->index_first[hash];
                     rv && (i <= t->index_last[hash]); ++i) {
                    if (elts[i].key && (checksum == elts[i].key_checksum) &&
                                        !strcasecmp(elts[i].key, argp)) {
                        rv = (*comp) (rec, elts[i].key, elts[i].val);
                    }
                }
            }
        }
        else {
            /* Scan the entire table */
            for (i = 0; rv && (i < t->a.nelts); ++i) {
                if (elts[i].key) {
                    rv = (*comp) (rec, elts[i].key, elts[i].val);
                }
            }
        }
        if (rv == 0) {
            vdorv = 0;
        }
    } while (argp && ((argp = va_arg(vp, char *)) != NULL));

    return vdorv;
}

static gbw_table_entry_t **table_mergesort(gbw_pool_t *pool,
                                           gbw_table_entry_t **values, 
                                           size_t n)
{
    /* Bottom-up mergesort, based on design in Sedgewick's "Algorithms
     * in C," chapter 8
     */
    gbw_table_entry_t **values_tmp =
        (gbw_table_entry_t **)gbw_palloc(pool, n * sizeof(gbw_table_entry_t*));
    size_t i;
    size_t blocksize;

    /* First pass: sort pairs of elements (blocksize=1) */
    for (i = 0; i + 1 < n; i += 2) {
        if (strcasecmp(values[i]->key, values[i + 1]->key) > 0) {
            gbw_table_entry_t *swap = values[i];
            values[i] = values[i + 1];
            values[i + 1] = swap;
        }
    }

    /* Merge successively larger blocks */
    blocksize = 2;
    while (blocksize < n) {
        gbw_table_entry_t **dst = values_tmp;
        size_t next_start;
        gbw_table_entry_t **swap;

        /* Merge consecutive pairs blocks of the next blocksize.
         * Within a block, elements are in sorted order due to
         * the previous iteration.
         */
        for (next_start = 0; next_start + blocksize < n;
             next_start += (blocksize + blocksize)) {

            size_t block1_start = next_start;
            size_t block2_start = block1_start + blocksize;
            size_t block1_end = block2_start;
            size_t block2_end = block2_start + blocksize;
            if (block2_end > n) {
                /* The last block may be smaller than blocksize */
                block2_end = n;
            }
            for (;;) {

                /* Merge the next two blocks:
                 * Pick the smaller of the next element from
                 * block 1 and the next element from block 2.
                 * Once either of the blocks is emptied, copy
                 * over all the remaining elements from the
                 * other block
                 */
                if (block1_start == block1_end) {
                    for (; block2_start < block2_end; block2_start++) {
                        *dst++ = values[block2_start];
                    }
                    break;
                }
                else if (block2_start == block2_end) {
                    for (; block1_start < block1_end; block1_start++) {
                        *dst++ = values[block1_start];
                    }
                    break;
                }
                if (strcasecmp(values[block1_start]->key,
                               values[block2_start]->key) > 0) {
                    *dst++ = values[block2_start++];
                }
                else {
                    *dst++ = values[block1_start++];
                }
            }
        }

        /* If n is not a multiple of 2*blocksize, some elements
         * will be left over at the end of the array.
         */
        for (i = dst - values_tmp; i < n; i++) {
            values_tmp[i] = values[i];
        }

        /* The output array of this pass becomes the input
         * array of the next pass, and vice versa
         */
        swap = values_tmp;
        values_tmp = values;
        values = swap;

        blocksize += blocksize;
    }

    return values;
}

void gbw_table_compress(gbw_table_t *t, unsigned flags)
{
    gbw_table_entry_t **sort_array;
    gbw_table_entry_t **sort_next;
    gbw_table_entry_t **sort_end;
    gbw_table_entry_t *table_next;
    gbw_table_entry_t **last;
    int i;
    int dups_found;

    if (t->a.nelts <= 1) {
        return;
    }

    /* Copy pointers to all the table elements into an
     * array and sort to allow for easy detection of
     * duplicate keys
     */
    sort_array = (gbw_table_entry_t **)
        gbw_palloc(t->a.pool, t->a.nelts * sizeof(gbw_table_entry_t*));
    sort_next = sort_array;
    table_next = (gbw_table_entry_t *)t->a.elts;
    i = t->a.nelts;
    do {
        *sort_next++ = table_next++;
    } while (--i);

    /* Note: the merge is done with mergesort instead of quicksort
     * because mergesort is a stable sort and runs in n*log(n)
     * time regardless of its inputs (quicksort is quadratic in
     * the worst case)
     */
    sort_array = table_mergesort(t->a.pool, sort_array, t->a.nelts);

    /* Process any duplicate keys */
    dups_found = 0;
    sort_next = sort_array;
    sort_end = sort_array + t->a.nelts;
    last = sort_next++;
    while (sort_next < sort_end) {
        if (((*sort_next)->key_checksum == (*last)->key_checksum) &&
            !strcasecmp((*sort_next)->key, (*last)->key)) {
            gbw_table_entry_t **dup_last = sort_next + 1;
            dups_found = 1;
            while ((dup_last < sort_end) &&
                   ((*dup_last)->key_checksum == (*last)->key_checksum) &&
                   !strcasecmp((*dup_last)->key, (*last)->key)) {
                dup_last++;
            }
            dup_last--; /* Elements from last through dup_last, inclusive,
                         * all have the same key
                         */
            if (flags == GBW_OVERLAP_TABLES_MERGE) {
                size_t len = 0;
                gbw_table_entry_t **next = last;
                char *new_val;
                char *val_dst;
                do {
                    len += strlen((*next)->val);
                    len += 2; /* for ", " or trailing null */
                } while (++next <= dup_last);
                new_val = (char *)gbw_palloc(t->a.pool, len);
                val_dst = new_val;
                next = last;
                for (;;) {
                    strcpy(val_dst, (*next)->val);
                    val_dst += strlen((*next)->val);
                    next++;
                    if (next > dup_last) {
                        *val_dst = 0;
                        break;
                    }
                    else {
                        *val_dst++ = ',';
                        *val_dst++ = ' ';
                    }
                }
                (*last)->val = new_val;
            }
            else { /* overwrite */
                (*last)->val = (*dup_last)->val;
            }
            do {
                (*sort_next)->key = NULL;
            } while (++sort_next <= dup_last);
        }
        else {
            last = sort_next++;
        }
    }

    /* Shift elements to the left to fill holes left by removing duplicates */
    if (dups_found) {
        gbw_table_entry_t *src = (gbw_table_entry_t *)t->a.elts;
        gbw_table_entry_t *dst = (gbw_table_entry_t *)t->a.elts;
        gbw_table_entry_t *last_elt = src + t->a.nelts;
        do {
            if (src->key) {
                *dst++ = *src;
            }
        } while (++src < last_elt);
        t->a.nelts -= (int)(last_elt - dst);
    }

    table_reindex(t);
}

static void gbw_table_cat(gbw_table_t *t, const gbw_table_t *s)
{
    const int n = t->a.nelts;
    register int idx;

    gbw_array_cat(&t->a,&s->a);

    if (n == 0) {
        memcpy(t->index_first,s->index_first,sizeof(int) * TABLE_HASH_SIZE);
        memcpy(t->index_last, s->index_last, sizeof(int) * TABLE_HASH_SIZE);
        t->index_initialized = s->index_initialized;
        return;
    }

    for (idx = 0; idx < TABLE_HASH_SIZE; ++idx) {
        if (TABLE_INDEX_IS_INITIALIZED(s, idx)) {
            t->index_last[idx] = s->index_last[idx] + n;
            if (!TABLE_INDEX_IS_INITIALIZED(t, idx)) {
                t->index_first[idx] = s->index_first[idx] + n;
            }
        }
    }

    t->index_initialized |= s->index_initialized;
}

void gbw_table_overlap(gbw_table_t *a, const gbw_table_t *b,
				    unsigned flags)
{
    if (a->a.nelts + b->a.nelts == 0) {
        return;
    }



    gbw_table_cat(a, b);

    gbw_table_compress(a, flags);
}

static int table_getm_do(void *v, const char *key, const char *val)
{
	key = key;

    table_getm_t *state = (table_getm_t *) v;

    if (!state->first) {
        /**
         * The most common case is a single header, and this is covered by
         * a fast path that doesn't allocate any memory. On the second and
         * subsequent header, an array is created and the array concatenated
         * together to form the final value.
         */
        state->first = val;
    }
    else {
        const char **elt;
        if (!state->merged) {
            state->merged = gbw_array_make(state->p, 10, sizeof(const char *));
            elt = gbw_array_push(state->merged);
            *elt = state->first;
        }
        elt = gbw_array_push(state->merged);
        *elt = val;
    }
    return 1;
}

const char * gbw_table_getm(gbw_pool_t *p, const gbw_table_t *t,
        const char *key)
{
    table_getm_t state;

    state.p = p;
    state.first = NULL;
    state.merged = NULL;

    gbw_table_do(table_getm_do, &state, t, key, NULL);

    if (!state.first) {
        return NULL;
    }
    else if (!state.merged) {
        return state.first;
    }
    else {
        return gbw_array_pstrcat(p, state.merged, ',');
    }
}

