#ifndef _LEX_H_
#define _LEX_H_

#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#define assert_token(x)       assert(match_token(x))
#define assert_token_name(x)  assert(token.name == str_intern(x) && \
                                                match_token(TOKEN_NAME))
#define assert_token_int(x)   assert(token.int_val == (x) && \
                                                match_token(TOKEN_INT))
#define assert_token_float(x) assert(token.float_val == (x) && \
                                                match_token(TOKEN_FLOAT))
#define assert_token_str(x)   assert(strcmp(token.str_val, (x)) == 0 && \
                                                match_token(TOKEN_STR))
#define assert_token_mod(x)   assert(token.mod == (x));
#define assert_token_eof()    assert(is_token(0))

#define CASE1(c, c1, k1)                    \
        case c:                             \
                token.kind = *stream++;     \
                if (*stream == c1) {        \
                        token.kind = k1;    \
                        ++stream;           \
                }                           \
                break;

#define CASE2(c, c1, k1, c2, k2)            \
        case c:                             \
                token.kind = *stream++;     \
                if (*stream == c1) {        \
                        token.kind = k1;    \
                        ++stream;           \
                } else if (*stream == c2) { \
                        token.kind = k2;    \
                        ++stream;           \
                }                           \
                break;

typedef enum TokenKind {
        TOKEN_EOF = 0,
        // Reserve first 128 values for one-char tokens.
        TOKEN_LAST_CHAR = 127,
        TOKEN_INT,
        TOKEN_FLOAT,
        TOKEN_STR,
        TOKEN_NAME,
        TOKEN_LSHIFT,
        TOKEN_RSHIFT,
        TOKEN_EQ,
        TOKEN_NOTEQ,
        TOKEN_LTEQ,
        TOKEN_GTEQ,
        TOKEN_AND,
        TOKEN_OR,
        TOKEN_INC,
        TOKEN_DEC,
        TOKEN_COLON_ASSIGN,
        TOKEN_ADD_ASSIGN,
        TOKEN_SUB_ASSIGN,
        TOKEN_OR_ASSIGN,
        TOKEN_AND_ASSIGN,
        TOKEN_XOR_ASSIGN,
        TOKEN_LSHIFT_ASSIGN,
        TOKEN_RSHIFT_ASSIGN,
        TOKEN_MUL_ASSIGN,
        TOKEN_DIV_ASSIGN,
        TOKEN_MOD_ASSIGN,
        TOKEN_KEYWORD
} TokenKind;

typedef enum TokenMod {
        TOKENMOD_NONE,
        TOKENMOD_BIN,
        TOKENMOD_OCT,
        TOKENMOD_HEX,
        TOKENMOD_CHAR
} TokenMod;

typedef struct Token {
        TokenKind kind;
        TokenMod mod;
        const char *start;
        const char *end;
        union {
                uint64_t int_val;
                double float_val;
                const char *str_val;
                const char *name;
        };
} Token;

Token token;
const char *stream;
const char *keyword_if;
const char *keyword_for;
const char *keyword_while;
extern uint8_t char_to_digit[256];
extern char escape_to_char[256];

void
init_keywords(void);

void
scan_int(void);

void
scan_float(void);

void
scan_char(void);

void
scan_str(void);

void
next_token(void);

void
init_stream(const char *str);

void
print_token(Token token);

bool
is_token(TokenKind kind);

bool
is_token_name(const char *name);

bool
match_token(TokenKind kind);

bool
expect_token(TokenKind kind);

size_t
copy_token_kind_str(char *dest, size_t dest_size, TokenKind kind);

const char *
token_kind_str(TokenKind kind);

void
lex_test(void);

#endif
