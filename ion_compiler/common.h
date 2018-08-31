#ifndef _COMMON_H_
#define _COMMON_H_

#include <assert.h>
#include <ctype.h>
#include <math.h>
#include <stdarg.h>
#include <stdio.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define buf__hdr(b)      ((BufHdr *) ((char *) (b) - offsetof(BufHdr, buf)))
#define buf__fits(b, n)  (buf_len(b) + (n) <= buf_cap(b))
#define buf__fit(b, n)   (buf__fits((b), (n)) ? 0 : ((b) = buf__grow((b), \
                                buf_len(b) + (n), sizeof(*(b)))))

#define buf_len(b)       ((b) ? buf__hdr(b)->len : 0)
#define buf_cap(b)       ((b) ? buf__hdr(b)->cap : 0)
#define buf_end(b)       ((b) + buf_len(b))
#define buf_free(b)      ((b) ? (free(buf__hdr(b)), (b) = NULL) : 0)
#define buf_push(b, ...) (buf__fit((b), 1), \
                                (b)[buf__hdr(b)->len++] = (__VA_ARGS__))

#define MAX(a, b) ((a) > (b) ? (a) : (b))

typedef struct {
        size_t len;
        size_t cap;
        char buf[0];
} BufHdr;

typedef struct Intern {
        size_t len;
        const char *str;
} Intern;

void *
xmalloc(size_t num_bytes);

void *
xrealloc(void *ptr, size_t num_bytes);

void
fatal(const char *fmt, ...);

void
syntax_error(const char *fmt, ...);

void *
buf__grow(const void *buf, size_t new_len, size_t elem_size);

void
buf_test(void);

const char *
str_intern_range(const char *start, const char *end);

const char *
str_intern(const char *str);

void
intern_test();

void
common_test();

#endif
