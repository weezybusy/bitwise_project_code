#include <assert.h>
#include <ctype.h>
#include <limits.h>
#include <stdarg.h>
#include <stdbool.h>
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

typedef struct Intern {
        size_t len;
        const char *str;
} Intern;

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
str_intern_test()
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

typedef enum TokenKind {
        END_OF_FILE = 0,
        // Reserve first 128 values for one-char tokens.
        TOKEN_LAST_CHAR = 127,
        TOKEN_INT,
        TOKEN_NAME
        // ...
} TokenKind;

size_t
copy_token_kind_str(char *dest, size_t dest_size, TokenKind kind)
{
        size_t n;

        n = 0;

        switch (kind) {
        case END_OF_FILE:
                n = snprintf(dest, dest_size, "end of file");
                break;
        case TOKEN_INT:
                n = snprintf(dest, dest_size, "integer");
                break;
        case TOKEN_NAME:
                n = snprintf(dest, dest_size, "name");
                break;
        default:
                if (kind < 128 && isprint(kind)) {
                        n = snprintf(dest, dest_size, "%c", kind);
                } else {
                        n = snprintf(dest, dest_size, "<ASCII %d>", kind);
                }
                break;
        }
        return n;
}

// Warning! This returns a pointer to a static internal buffer, so the next
// call will overwrite it.
const char *
token_kind_str(TokenKind kind)
{
        static char buf[256];
        size_t n;

        n = copy_token_kind_str(buf, sizeof(buf), kind);
        assert(n + 1 <= sizeof(buf));
        return buf;
}

typedef struct Token {
        TokenKind kind;
        const char *start;
        const char *end;
        union {
                uint64_t int_val;
                const char *name;
        };
} Token;

Token token;
const char *stream;

const char *keyword_if;
const char *keyword_for;
const char *keyword_while;

void
init_keywords(void)
{
        keyword_if = str_intern("if");
        keyword_for = str_intern("for");
        keyword_while = str_intern("while");
        // ...
}

uint8_t char_to_digit[256] = {
        ['0'] = 0,
        ['1'] = 1,
        ['2'] = 2,
        ['3'] = 3,
        ['4'] = 4,
        ['5'] = 5,
        ['6'] = 6,
        ['7'] = 7,
        ['8'] = 8,
        ['9'] = 9,
        ['a'] = 10, ['A'] = 10,
        ['b'] = 11, ['B'] = 11,
        ['c'] = 12, ['C'] = 12,
        ['d'] = 13, ['D'] = 13,
        ['e'] = 14, ['E'] = 14,
        ['f'] = 15, ['F'] = 15
};

uint64_t
scan_int(void)
{
        uint64_t base;
        uint64_t val;
        uint64_t digit;

        base = 10;
        val = 0;

        if (*stream == '0') {
                ++stream;
                if (tolower(*stream) == 'x') {
                        ++stream;
                        base = 16;
                } else if (tolower(*stream) == 'b') {
                        ++stream;
                        base = 2;
                } else if (isdigit(*stream)) {
                        base = 8;
                } else {
                        syntax_error("Invalid integer literal suffix '%c'",
                                        *stream);
                        ++stream;
                }
        }

        while (1) {
                digit = char_to_digit[(int) *stream];
                if (digit == 0 && *stream != '0') {
                        break;
                }

                if (digit >= base) {
                        syntax_error("Digit '%c' out of range for base %llu",
                                        *stream, base);
                        digit = 0;
                }

                if (val > (UINT64_MAX - digit)/base) {
                        syntax_error("Integer literal overflow");
                        while (isdigit(*stream)) {
                                ++stream;
                        }
                        val = 0;
                }
                val = val * base + digit;
                ++stream;
        }
        return val;
}

void
next_token(void)
{
        token.start = stream;

        while (isspace(*stream)) {
                ++stream;
        }

        switch (*stream) {
        case '0': case '1': case '2': case '3': case '4': case '5': case '6':
        case '7': case '8': case '9':
                token.kind = TOKEN_INT;
                token.int_val = scan_int();
                break;
        case 'a': case 'b': case 'c': case 'd': case 'e': case 'f': case 'g':
        case 'h': case 'i': case 'j': case 'k': case 'l': case 'm': case 'n':
        case 'o': case 'p': case 'q': case 'r': case 's': case 't': case 'u':
        case 'v': case 'w': case 'x': case 'y': case 'z':
        case 'A': case 'B': case 'C': case 'D': case 'E': case 'F': case 'G':
        case 'H': case 'I': case 'J': case 'K': case 'L': case 'M': case 'N':
        case 'O': case 'P': case 'Q': case 'R': case 'S': case 'T': case 'U':
        case 'V': case 'W': case 'X': case 'Y': case 'Z': case '_':
                while (isalnum(*stream) || *stream == '_')
                        stream++;
                token.kind = TOKEN_NAME;
                token.name = str_intern_range(token.start, stream);
                break;
        default:
                token.kind = *stream++;
                break;
        }

        token.end = stream;
}

void
init_stream(const char *str)
{
        stream = str;
        next_token();
}

void
print_token(Token token)
{
        switch (token.kind) {
        case TOKEN_INT:
                printf("TOKEN INT: %lu\n", token.int_val);
                break;
        case TOKEN_NAME:
                printf("TOKEN NAME: %.*s\n",
                                (int) (token.end - token.start), token.start);
                break;
        default:
                printf("TOKEN '%c'\n", token.kind);
                break;                        
        }
}

bool
is_token(TokenKind kind)
{
        return token.kind == kind;
}

bool
is_token_name(const char *name)
{
        return token.kind == TOKEN_NAME && token.name == name;
}

bool
match_token(TokenKind kind)
{
        if (is_token(kind)) {
                next_token();
                return true;
        } else {
                return false;
        }
}

bool
expect_token(TokenKind kind)
{
        char buf[256];

        if (is_token(kind)) {
                next_token();
                return true;
        } else {
                copy_token_kind_str(buf, sizeof(buf), kind);
                fatal("expected token %s got %s\n", buf,
                                token_kind_str(token.kind));
                return false;
        }
}

#define assert_token(x)      assert(match_token(x))
#define assert_token_name(x) assert(token.name == str_intern(x) && \
                                                match_token(TOKEN_NAME))
#define assert_token_int(x)  assert(token.int_val == (x) && \
                                                match_token(TOKEN_INT))
#define assert_token_eof()   assert(is_token(0))

void
lex_test(void)
{
        // Integer literal tests.
        init_stream("18446744073709551615 0xffffffffffffffff 042 0b1111");
        assert_token_int(18446744073709551615ULL);
        assert_token_int(0xffffffffffffffffULL);
        assert_token_int(042);
        assert_token_int(0xf);
        assert_token_eof();

        const char *str = "XY+(XY)_HELLO1,234+994";
        init_stream(str);
        assert_token_name("XY");
        assert_token('+');
        assert_token('(');
        assert_token_name("XY");
        assert_token(')');
        assert_token_name("_HELLO1");
        assert_token(',');
        assert_token_int(234);
        assert_token('+');
        assert_token_int(994);
        assert_token_eof();
}

#undef assert_token
#undef assert_token_name
#undef assert_token_int
#undef assert_token_eof

void
run_tests(void)
{
        buf_test();
        lex_test();
        str_intern_test();
        // parse_test();
}

int
main(int argc, char **argv)
{
        run_tests();
        return 0;
}
