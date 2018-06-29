#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define MAX(a, b) ((a) > (b) ? (a) : (b))

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

#define buf__hdr(b)     ((BufHdr *)((char *)(b) - offsetof(BufHdr, buf)))
#define buf__fits(b, n) (buf_len(b) + (n) <= buf_cap(b))
#define buf__fit(b, n)  (buf__fits((b), (n)) ? 0 : ((b) = buf__grow((b), buf_len(b) + (n), sizeof(*(b)))))

#define buf_len(b)      ((b) ? buf__hdr(b)->len : 0)
#define buf_cap(b)      ((b) ? buf__hdr(b)->cap : 0)
#define buf_push(b, x)  (buf__fit((b), 1), (b)[buf__hdr(b)->len++] = (x))
#define buf_free(b)     ((b) ? (free(buf__hdr(b)), (b) = NULL) : 0)

typedef struct {
        size_t len;
        size_t cap;
        char buf[0];
} BufHdr;

void *
buf__grow(const void *buf, size_t new_len, size_t elem_size)
{
        size_t new_cap;
        size_t new_size;
        BufHdr *new_hdr;

        new_cap = MAX(1 + 2 * buf_cap(buf), new_len);
        new_size = (new_cap * elem_size) + offsetof(BufHdr, buf);

        if (buf)
                new_hdr = xrealloc(buf__hdr(buf), new_size);
        else {
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

        buf = NULL;
        assert(buf_len(buf) == 0);

        enum { N = 1024 };

        for (int i = 0; i < N; ++i) {
                buf_push(buf, i);
        }
        assert(buf_len(buf) == N);

        for (int i = 0; i < N; ++i)
                assert(buf[i] == i);

        buf_free(buf);
        assert(buf == NULL);
        assert(buf_len(buf) == 0);
}

typedef struct InternStr {
        size_t len;
        const char *str;
} InternStr;

static InternStr *interns;

const char *
str_intern_range(const char *start, const char *end)
{
        size_t len;
        char *str;

        len = end - start;
        for (size_t i = 0; i < buf_len(interns); ++i) {
                if (interns[i].len == len && strncmp(interns[i].str, start, len) == 0) {
                        return interns[i].str;
                }
        }

        str = xmalloc(len + 1);
        memcpy(str, start, len);
        str[len] = 0;
        buf_push(interns, ((InternStr) { len, str }));
        return str;
}

const char *
str_intern(const char *str)
{
        return str_intern_range(str, str + strlen(str));
}

void
str_intern_test()
{
        char x[] = "hello";
        char y[] = "hello";

        assert(x != y);
        //assert(str_intern(x) == str_intern(y));
}

typedef enum TokenKind {
        TOKEN_INT = 128,
        TOKEN_NAME
} TokenKind;

typedef struct Token {
        TokenKind kind;
        const char *start;
        const char *end;
        union {
                uint64_t val;
        };
} Token;

Token token;
const char *stream;

void
next_token(void)
{
        uint64_t val;

        token.start = stream;
        switch (*stream) {
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
                val = 0;
                while (isdigit(*stream))
                        val = 10 * val + (*stream++ - '0');
                token.kind = TOKEN_INT;
                token.val = val;
                break;
        case 'a':
        case 'b':
        case 'c':
        case 'd':
        case 'e':
        case 'f':
        case 'g':
        case 'h':
        case 'i':
        case 'j':
        case 'k':
        case 'l':
        case 'm':
        case 'n':
        case 'o':
        case 'p':
        case 'q':
        case 'r':
        case 's':
        case 't':
        case 'u':
        case 'v':
        case 'w':
        case 'x':
        case 'y':
        case 'z':
        case 'A':
        case 'B':
        case 'C':
        case 'D':
        case 'E':
        case 'F':
        case 'G':
        case 'H':
        case 'I':
        case 'J':
        case 'K':
        case 'L':
        case 'M':
        case 'N':
        case 'O':
        case 'P':
        case 'Q':
        case 'R':
        case 'S':
        case 'T':
        case 'U':
        case 'V':
        case 'W':
        case 'X':
        case 'Y':
        case 'Z':
        case '_':
                token.start = stream;
                while (isalnum(*stream) || *stream == '_')
                        stream++;
                token.end = stream;
                token.kind = TOKEN_NAME;
                break;
        default:
                token.kind = *stream++;
                break;
        }
        token.end = stream;
}

void
print_token(Token token)
{
        switch (token.kind) {
                case TOKEN_INT:
                        printf("TOKEN INT: %lu\n", token.val);
                        break;
                case TOKEN_NAME:
                        printf("TOKEN NAME: %.*s\n",
                                        (int) (token.end - token.start),
                                        token.start);
                        break;
                default:
                        printf("TOKEN '%c'\n", token.kind);
                        break;                        
        }
}

void
lex_test(void)
{
        char *source = "+()_HELLO1,234+FOO!567";
        stream = source;
        next_token();
        while (token.kind) {
                print_token(token);
                next_token();
        }
}

int
main(int argc, char **argv)
{
        buf_test();
        lex_test();
        str_intern_test();

        return 0;
}
