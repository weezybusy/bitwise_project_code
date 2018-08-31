#include "common.h"

void *
xmalloc(size_t num_bytes)
{
        void *ptr;

        ptr = malloc(num_bytes);
        if (ptr == NULL) {
                perror("xmalloc: failed to allocate memory");
                exit(EXIT_FAILURE);
        }
        return ptr;
}

void *
xrealloc(void *ptr, size_t num_bytes)
{
        ptr = realloc(ptr, num_bytes);
        if (ptr == NULL) {
                perror("xrealloc: failed to reallocate memory");
                exit(EXIT_FAILURE);
        }
        return ptr;
}

void
fatal(const char *fmt, ...)
{
        va_list args;
        va_start(args, fmt);
        printf("FATAL: ");
        vprintf(fmt, args);
        printf("\n");
        va_end(args);
        exit(1);
}

void
syntax_error(const char *fmt, ...)
{
        va_list args;
        va_start(args, fmt);
        printf("SYNTAX ERROR: ");
        vprintf(fmt, args);
        printf("\n");
        va_end(args);
        exit(1);
}

void *
buf__grow(const void *buf, size_t new_len, size_t elem_size)
{
        size_t new_cap;
        size_t new_size;
        BufHdr *new_hdr;

        assert(buf_cap(buf) <= (SIZE_MAX - 1) / 2);
        new_cap = MAX(1 + 2 * buf_cap(buf), new_len);
        assert(new_len <= new_cap);
        assert(new_cap <= (SIZE_MAX - offsetof(BufHdr, buf)) / elem_size);
        new_size = (new_cap * elem_size) + offsetof(BufHdr, buf);

        if (buf) {
                new_hdr = xrealloc(buf__hdr(buf), new_size);
        } else {
                new_hdr = xmalloc(new_size);
                new_hdr->len = 0;
        }
        new_hdr->cap = new_cap;

        return new_hdr->buf;
}

void
buf_test(void)
{
        int *buf;
        int n;

        buf = NULL;
        n = 1024;

        assert(buf_len(buf) == 0);

        for (int i = 0; i < n; ++i) {
                buf_push(buf, i);
        }

        assert(buf_len(buf) == n);

        for (int i = 0; i < buf_len(buf); ++i)
                assert(buf[i] == i);

        buf_free(buf);
        assert(buf == NULL);
        assert(buf_len(buf) == 0);
}

static Intern *interns;

const char *
str_intern_range(const char *start, const char *end)
{
        size_t len;
        char *str;

        len = end - start;
        for (Intern *it = interns; it != buf_end(interns); ++it) {
                if (it->len == len && strncmp(it->str, start, len) == 0) {
                        return it->str;
                }
        }

        str = xmalloc(len + 1);
        memcpy(str, start, len);
        str[len] = 0;
        buf_push(interns, (Intern) { len, str });

        return str;
}

const char *
str_intern(const char *str)
{
        return str_intern_range(str, str + strlen(str));
}

void
intern_test()
{
        char x[] = "hello";
        char y[] = "hello";
        assert(x != y);
        const char *px = str_intern(x);
        const char *py = str_intern(y);
        assert(px == py);

        char z[] = "hello!";
        const char *pz = str_intern(z);
        assert(pz != px);
}

void
common_test()
{
        buf_test();
        intern_test();
}
