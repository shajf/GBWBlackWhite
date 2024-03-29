/*
 *
 *      Filename: gbw_string.h
 *
 *        Author: shajf,csp001314@163.com
 *   Description: ---
 *        Create: 2017-01-09 12:09:56
 * Last Modified: 2017-01-09 12:09:56
 */

#ifndef GBW_STRING_H
#define GBW_STRING_H

typedef struct gbw_vformatter_buff_t gbw_vformatter_buff_t;                                                                     

#include "gbw_mpool.h"
#include <ctype.h>
#include <sys/uio.h>
#include <stdarg.h>

typedef struct {
    size_t len;
    unsigned char *data;
}gbw_str_t;

#define gbw_string(str) { sizeof(str) - 1, str}
#define gbw_string_empty(str) ((str)==NULL||strlen(str)==0)

#define VALID_HEX(X) (((X >= '0')&&(X <= '9')) || ((X >= 'a')&&(X <= 'f')) || ((X >= 'A')&&(X <= 'F')))
#define NBSP 160
#define HUGE_STRING_LEN 8192                          
#define ISODIGIT(X) ((X >= '0')&&(X <= '7'))

#define gbw_isalpha(c) (isalpha(((unsigned char)(c))))
#define gbw_isdigit(c) (isdigit(((unsigned char)(c))))
#define gbw_islower(c) (islower(((unsigned char)(c))))
#define gbw_isspace(c) (isspace(((unsigned char)(c))))
#define gbw_toupper(c) (toupper(((unsigned char)(c))))

#define GBW_SSIZE_T_FMT "ld"                                                                                                    
                                                                                                                                 
/*  And GBW_SIZE_T_FMT */                                                                                                        
#define GBW_SIZE_T_FMT "lu"                                                                                                     
                                                                                                                                 
/*  And GBW_OFF_T_FMT */                                                                                                         
#define GBW_OFF_T_FMT "ld"                                                                                                      
                                                                                                                                 
/*  And GBW_PID_T_FMT */                                                                                                         
#define GBW_PID_T_FMT "d"                                                                                                       
                                                                                                                                 
/*  And GBW_INT64_T_FMT */                                                                                                       
#define GBW_INT64_T_FMT "ld"                                                                                                    
                                                                                                                                 
/*  And GBW_UINT64_T_FMT */                                                                                                      
#define GBW_UINT64_T_FMT "lu"                                                                                                   
                                                                                                                                 
/*  And GBW_UINT64_T_HEX_FMT */                                                                                                  
#define GBW_UINT64_T_HEX_FMT "lx"

/**                                                                                                                             
* Structure used by the variable-formatter routines.                                                                           
*/                                                                                                                             
struct gbw_vformatter_buff_t {                                                                                                  
  /** The current position */                                                                                                 
  char *curpos;                                                                                                               
  /** The end position of the format string */                                                                                
  char *endpos;                                                                                                               
};                                                                                                                              
	

/**
 * Do a natural order comparison of two strings.
 * @param a The first string to compare
 * @param b The second string to compare
 * @return Either <0, 0, or >0.  If the first string is less than the second
 *          this returns <0, if they are equivalent it returns 0, and if the
 *          first string is greater than second string it retuns >0.
 */
extern int gbw_strnatcmp(const char *a, const char *b);

/**
 * Do a natural order comparison of two strings ignoring the case of the 
 * strings.
 * @param a The first string to compare
 * @param b The second string to compare
 * @return Either <0, 0, or >0.  If the first string is less than the second
 *         this returns <0, if they are equivalent it returns 0, and if the
 *         first string is greater than second string it retuns >0.
 */
extern int gbw_strnatcasecmp(const char *a, const char *b);

/**
 * duplicate a string into memory allocated out of a pool
 * @param p The pool to allocate out of
 * @param s The string to duplicate
 * @return The new string or NULL if s == NULL
 */
extern char * gbw_pstrdup(gbw_pool_t *p, const char *s);

/**
 * Create a null-terminated string by making a copy of a sequence
 * of characters and appending a null byte
 * @param p The pool to allocate out of
 * @param s The block of characters to duplicate
 * @param n The number of characters to duplicate
 * @return The new string or NULL if s == NULL
 * @remark This is a faster alternative to gbw_pstrndup, for use
 *         when you know that the string being duplicated really
 *         has 'n' or more characters.  If the string might contain
 *         fewer characters, use gbw_pstrndup.
 */
extern char * gbw_pstrmemdup(gbw_pool_t *p, const char *s, size_t n);


/**
 * Duplicate at most n characters of a string into memory allocated 
 * out of a pool; the new string will be NUL-terminated
 * @param p The pool to allocate out of
 * @param s The string to duplicate
 * @param n The maximum number of characters to duplicate
 * @return The new string or NULL if s == NULL
 * @remark The amount of memory allocated from the pool is the length
 *         of the returned string including the NUL terminator
 */
extern char * gbw_pstrndup(gbw_pool_t *p, const char *s, size_t n);

/**
 * Duplicate a block of memory.
 *
 * @param p The pool to allocate from
 * @param m The memory to duplicate
 * @param n The number of bytes to duplicate
 * @return The new block of memory or NULL if m == NULL
 */
extern void * gbw_pmemdup(gbw_pool_t *p, const void *m, size_t n);

/**
 * Concatenate multiple strings, allocating memory out a pool
 * @param p The pool to allocate out of
 * @param ... The strings to concatenate.  The final string must be NULL
 * @return The new string
 */
extern char * gbw_pstrcat(gbw_pool_t *p, ...);

/**
 * Concatenate multiple strings specified in a writev-style vector
 * @param p The pool from which to allocate
 * @param vec The strings to concatenate
 * @param nvec The number of strings to concatenate
 * @param nbytes (output) strlen of new string (pass in NULL to omit)
 * @return The new string
 */
extern char * gbw_pstrcatv(gbw_pool_t *p, const struct iovec *vec,
                                 size_t nvec, size_t *nbytes);


/**
 * printf-style style printing routine.  The data is output to a string 
 * allocated from a pool
 * @param p The pool to allocate out of
 * @param fmt The format of the string
 * @param ... The arguments to use while printing the data
 * @return The new string
 */

/**
 * Copy up to dst_size characters from src to dst; does not copy
 * past a NUL terminator in src, but always terminates dst with a NUL
 * regardless.
 * @param dst The destination string
 * @param src The source string
 * @param dst_size The space available in dst; dst always receives
 *                 NUL termination, so if src is longer than
 *                 dst_size, the actual number of characters copied is
 *                 dst_size - 1.
 * @return Pointer to the NUL terminator of the destination string, dst
 * @remark
 * <PRE>
 * Note the differences between this function and strncpy():
 *  1) strncpy() doesn't always NUL terminate; gbw_cpystrn() does.
 *  2) strncpy() pads the destination string with NULs, which is often 
 *     unnecessary; gbw_cpystrn() does not.
 *  3) strncpy() returns a pointer to the beginning of the dst string;
 *     gbw_cpystrn() returns a pointer to the NUL terminator of dst, 
 *     to allow a check for truncation.
 * </PRE>
 */
extern char * gbw_cpystrn(char *dst, const char *src,
                                size_t dst_size);

/**
 * Remove all whitespace from a string
 * @param dest The destination string.  It is okay to modify the string
 *             in place.  Namely dest == src
 * @param src The string to rid the spaces from.
 * @return A pointer to the destination string's null terminator.
 */
extern char * gbw_collapse_spaces(char *dest, const char *src);

/**
 * Convert the arguments to a program from one string to an array of 
 * strings terminated by a NULL pointer
 * @param arg_str The arguments to convert
 * @param argv_out Output location.  This is a pointer to an array of strings.
 * @param token_context Pool to use.
 */
extern int gbw_tokenize_to_argv(const char *arg_str,
                                               char ***argv_out,
                                               gbw_pool_t *token_context);

/**
 * Split a string into separate null-terminated tokens.  The tokens are 
 * delimited in the string by one or more characters from the sep
 * argument.
 * @param str The string to separate; this should be specified on the
 *            first call to gbw_strtok() for a given string, and NULL
 *            on subsequent calls.
 * @param sep The set of delimiters
 * @param last State saved by gbw_strtok() between calls.
 * @return The next token from the string
 * @note the 'last' state points to the trailing NUL char of the final
 * token, otherwise it points to the character following the current
 * token (all successive or empty occurances of sep are skiped on the
 * subsequent call to gbw_strtok).  Therefore it is possible to avoid
 * a strlen() determination, with the following logic;
 * toklen = last - retval; if (*last) --toklen;
 */
extern char * gbw_strtok(char *str, const char *sep, char **last);


extern int gbw_strsplit(char *string, int stringlen,char **tokens, int maxtokens, char delim);
                                                                            
/**
 * @defgroup GBW_Strings_Snprintf snprintf implementations
 * @warning
 * These are snprintf implementations based on gbw_vformatter().
 *
 * Note that various standards and implementations disagree on the return
 * value of snprintf, and side-effects due to %n in the formatting string.
 * gbw_snprintf (and gbw_vsnprintf) behaves as follows:
 *
 * Process the format string until the entire string is exhausted, or
 * the buffer fills.  If the buffer fills then stop processing immediately
 * (so no further %n arguments are processed), and return the buffer
 * length.  In all cases the buffer is NUL terminated. It will return the
 * number of characters inserted into the buffer, not including the
 * terminating NUL. As a special case, if len is 0, gbw_snprintf will
 * return the number of characters that would have been inserted if
 * the buffer had been infinite (in this case, *buffer can be NULL)
 *
 * In no event does gbw_snprintf return a negative number.
 * @{
 */

extern int gbw_vformatter(int (*flush_func)(gbw_vformatter_buff_t *),
    gbw_vformatter_buff_t *vbuff, const char *fmt, va_list ap);

/**
 * snprintf routine based on gbw_vformatter.  This means it understands the
 * same extensions.
 * @param buf The buffer to write to
 * @param len The size of the buffer
 * @param format The format string
 * @param ... The arguments to use to fill out the format string.
 */
extern int gbw_snprintf(char *buf, size_t len,
                                     const char *format, ...)
        __attribute__((format(printf,3,4)));

char * gbw_psprintf(gbw_pool_t *p, const char *fmt, ...);

char * gbw_pvsprintf(gbw_pool_t *pool, const char *fmt, va_list ap);

/**
 * vsnprintf routine based on gbw_vformatter.  This means it understands the
 * same extensions.
 * @param buf The buffer to write to
 * @param len The size of the buffer
 * @param format The format string
 * @param ap The arguments to use to fill out the format string.
 */
extern int gbw_vsnprintf(char *buf, size_t len, const char *format,
                               va_list ap);
/** @} */

/**
 * create a string representation of an int, allocated from a pool
 * @param p The pool from which to allocate
 * @param n The number to format
 * @return The string representation of the number
 */
extern char * gbw_itoa(gbw_pool_t *p, int n);

/**
 * create a string representation of a long, allocated from a pool
 * @param p The pool from which to allocate
 * @param n The number to format
 * @return The string representation of the number
 */
extern char * gbw_ltoa(gbw_pool_t *p, long n);

/**
 * create a string representation of an off_t, allocated from a pool
 * @param p The pool from which to allocate
 * @param n The number to format
 * @return The string representation of the number
 */
extern char * off_t_toa(gbw_pool_t *p, off_t n);

/**
 * Convert a numeric string into an off_t numeric value.
 * @param offset The value of the parsed string.
 * @param buf The string to parse. It may contain optional whitespace,
 *   followed by an optional '+' (positive, default) or '-' (negative)
 *   character, followed by an optional '0x' prefix if base is 0 or 16,
 *   followed by numeric digits appropriate for base.
 * @param end A pointer to the end of the valid character in buf. If
 *   not NULL, it is set to the first invalid character in buf.
 * @param base A numeric base in the range between 2 and 36 inclusive,
 *   or 0.  If base is zero, buf will be treated as base ten unless its
 *   digits are prefixed with '0x', in which case it will be treated as
 *   base 16.
 * @bug *end breaks type safety; where *buf is const, *end needs to be
 * declared as const in GW 2.0
 */
extern int gbw_strtoff(off_t *offset, const char *buf, 
                                      char **end, int base);

/**
 * parse a numeric string into a 64-bit numeric value
 * @param buf The string to parse. It may contain optional whitespace,
 *   followed by an optional '+' (positive, default) or '-' (negative)
 *   character, followed by an optional '0x' prefix if base is 0 or 16,
 *   followed by numeric digits appropriate for base.
 * @param end A pointer to the end of the valid character in buf. If
 *   not NULL, it is set to the first invalid character in buf.
 * @param base A numeric base in the range between 2 and 36 inclusive,
 *   or 0.  If base is zero, buf will be treated as base ten unless its
 *   digits are prefixed with '0x', in which case it will be treated as
 *   base 16.
 * @return The numeric value of the string.  On overflow, errno is set
 * to ERANGE.  On success, errno is set to 0.
 */
extern int64_t gbw_strtoi64(const char *buf, char **end, int base);

/**
 * parse a base-10 numeric string into a 64-bit numeric value.
 * Equivalent to gbw_strtoi64(buf, (char**)NULL, 10).
 * @param buf The string to parse
 * @return The numeric value of the string.  On overflow, errno is set
 * to ERANGE.  On success, errno is set to 0.
 */
extern int64_t gbw_atoi64(const char *buf);

/**
 * Format a binary size (magnitiudes are 2^10 rather than 10^3) from an off_t,
 * as bytes, K, M, T, etc, to a four character compacted human readable string.
 * @param size The size to format
 * @param buf The 5 byte text buffer (counting the trailing null)
 * @return The buf passed to gbw_strfsize()
 * @remark All negative sizes report '  - ', gbw_strfsize only formats positive values.
 */
extern char * gbw_strfsize(off_t size, char *buf);

extern const char * gbw_filepath_name_get(const char *pathname);

/**
 * Converts a single byte into its hexadecimal representation.
 * Will overwrite two bytes at the destination.
 */
static inline unsigned char *gbw_c2x(unsigned what, unsigned char *where) {
    static const char c2x_table[] = "0123456789abcdef";

    what = what & 0xff;
    *where++ = c2x_table[what >> 4];
    *where++ = c2x_table[what & 0x0f];

    return where;
}

/**
 * Converts a byte given as its hexadecimal representation
 * into a proper byte. Handles uppercase and lowercase letters
 * but does not check for overflows.
 */
static inline unsigned char gbw_x2c(unsigned char *what) {
    register unsigned char digit;

    digit = (what[0] >= 'A' ? ((what[0] & 0xdf) - 'A') + 10 : (what[0] - '0'));
    digit *= 16;
    digit += (what[1] >= 'A' ? ((what[1] & 0xdf) - 'A') + 10 : (what[1] - '0'));

    return digit;
}

/**
 * Converts a single hexadecimal digit into a decimal value.
 */
static inline unsigned char gbw_xsingle2c(unsigned char *what) {
    register unsigned char digit;

    digit = (what[0] >= 'A' ? ((what[0] & 0xdf) - 'A') + 10 : (what[0] - '0'));

    return digit;
}

static inline char* gbw_last_char_is(const char *s, int c)
{
    if (s && *s) {
        size_t sz = strlen(s) - 1;
        s += sz;
        if ( (unsigned char)*s == c)
            return (char*)s;
    }
    return NULL;
}

static inline int gbw_is_empty_string(const char *string) {
    unsigned int i;

    if (string == NULL) return 1;

    for(i = 0; string[i] != '\0'; i++) {
        if (!isspace(string[i])) {
            return 0;
        }
    }

    return 1;
}

extern int gbw_string_endsWith(const char *target,const char *match);

extern int gbw_decode_base64(gbw_str_t *dst, gbw_str_t *src);

extern char *gbw_strlcasestrn(char *s1, char *last, char *s2, size_t n);

extern int gbw_strncasecmp(char *s1,char *s2,size_t n);

#endif /* GBW_STRING_H */
