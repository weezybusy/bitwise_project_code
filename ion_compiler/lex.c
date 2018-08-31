#include "lex.h"
#include "common.h"

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

char escape_to_char[256] = {
        ['n'] = '\n',
        ['r'] = '\r',
        ['t'] = '\t',
        ['v'] = '\v',
        ['b'] = '\b',
        ['a'] = '\a',
        ['0'] = 0
};

size_t
copy_token_kind_str(char *dest, size_t dest_size, TokenKind kind)
{
        size_t n;

        n = 0;

        switch (kind) {
        case TOKEN_EOF:
                n = snprintf(dest, dest_size, "end of file");
                break;
        case TOKEN_INT:
                n = snprintf(dest, dest_size, "integer");
                break;
        case TOKEN_FLOAT:
                n = snprintf(dest, dest_size, "float");
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

void
init_keywords(void)
{
        keyword_if = str_intern("if");
        keyword_for = str_intern("for");
        keyword_while = str_intern("while");
        // ...
}

void
scan_int(void)
{
        uint64_t base;
        uint64_t val;
        uint64_t digit;

        base = 10;
        val = 0;

        if (*stream == '0') {
                ++stream;
                if (tolower(*stream) == 'b') {
                        ++stream;
                        base = 2;
                        token.mod = TOKENMOD_BIN;
                } else if (isdigit(*stream)) {
                        base = 8;
                        token.mod = TOKENMOD_OCT;
                } else if (tolower(*stream) == 'x') {
                        ++stream;
                        base = 16;
                        token.mod = TOKENMOD_HEX;
                }
        }

        while (1) {
                digit = char_to_digit[(int) *stream];

                if (digit == 0 && *stream != '0') {
                        break;
                }

                if (digit >= base) {
                        syntax_error("Digit '%c' out of range for base %llu.",
                                        *stream, base);
                        digit = 0;
                }

                if (val > (UINT64_MAX - digit)/base) {
                        syntax_error("Integer literal overflow.");
                        while (isdigit(*stream)) {
                                ++stream;
                        }
                        val = 0;
                }
                val = val * base + digit;
                ++stream;
        }

        token.kind = TOKEN_INT;
        token.int_val = val;
}

void
scan_float(void)
{
        const char *start;
        double val;

        start = stream;

        while (isdigit(*stream)) {
                ++stream;
        }

        if (*stream == '.') {
                ++stream;
        }

        while (isdigit(*stream)) {
                ++stream;
        }

        if (tolower(*stream) == 'e') {
                ++stream;
                if (*stream == '-' || *stream == '+') {
                        ++stream;
                }
                if (!isdigit(*stream)) {
                        syntax_error("Expected digit after float literal "
                                        "exponent, found '%c'.", *stream);
                }
                while (isdigit(*stream)) {
                        ++stream;
                }
        }

        val = strtod(start, NULL);
        if (val == HUGE_VAL || val == -HUGE_VAL) {
                syntax_error("Float literal overflow.");
        }

        token.kind = TOKEN_FLOAT;
        token.float_val = val;
}

void
scan_char(void)
{
        char val;

        assert(*stream == '\'');
        ++stream;

        if (*stream == '\'') {
                syntax_error("Char literal cannot be empty.");
        } else if (*stream == '\n') {
                syntax_error("Char literal cannot contain newline.");
        } else if (*stream == '\\') {
                ++stream;
                val = escape_to_char[(int) *stream];
                if (val == '\0' && *stream != '0') {
                        syntax_error("Invalid char literal escape '\\%c'.",
                                        *stream);
                }
                ++stream;
        } else {
                val = *stream;
                ++stream;
        }

        if (*stream != '\'') {
                syntax_error("Expected closing char quote, got '%c'.",
                                *stream);
        } else {
                ++stream;
        }

        token.kind = TOKEN_INT;
        token.int_val = val;
        token.mod = TOKENMOD_CHAR;
}

void
scan_str(void)
{
        char val;
        char *str;

        str = NULL;
        assert(*stream == '\"');
        ++stream;

        while (*stream && *stream != '\"') {
                val = *stream;
                if (val == '\n') {
                        syntax_error("String literal cannot contain newline.");
                } else if (val == '\\') {
                        ++stream;
                        val = escape_to_char[(int) *stream];
                        if (val == 0 && *stream != '0') {
                                syntax_error("Invalid string literal escape "
                                                "'\\%c'.", *stream);
                        }
                }
                buf_push(str, val);
                ++stream;
        }

        if (*stream) {
                assert(*stream == '\"');
                ++stream;
        } else {
                syntax_error("Unexpected end of file within string literal.");
        }

        buf_push(str, 0);
        token.kind = TOKEN_STR;
        token.str_val = str;
}

void
next_token(void)
{
        char c;

        while (isspace(*stream)) {
                ++stream;
        }

        token.start = stream;
        token.mod = TOKENMOD_NONE;

        switch (*stream) {
        case '\'':
                scan_char();
                break;
        case '"':
                scan_str();
                break;
        case '.':
                scan_float();
                break;
        case '0': case '1': case '2': case '3': case '4': case '5': case '6':
        case '7': case '8': case '9':
                while (isdigit(*stream)) {
                        ++stream;
                }
                c = *stream;
                stream = token.start;
                if (c == '.' || tolower(c) == 'e') {
                        scan_float();
                } else {
                        scan_int();
                }
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
                        ++stream;
                token.kind = TOKEN_NAME;
                token.name = str_intern_range(token.start, stream);
                break;
        case '<':
                token.kind = *stream++;
                if (*stream == '<') {
                        token.kind = TOKEN_LSHIFT;
                        ++stream;
                        if (*stream == '=') {
                                token.kind = TOKEN_LSHIFT_ASSIGN;
                                ++stream;
                        }
                } else if (*stream == '=') {
                        token.kind = TOKEN_LTEQ;
                        ++stream;
                }
                break;
        case '>':
                token.kind = *stream++;
                if (*stream == '>') {
                        token.kind = TOKEN_RSHIFT;
                        ++stream;
                        if (*stream == '=') {
                                token.kind = TOKEN_RSHIFT_ASSIGN;
                                ++stream;
                        }
                } else if (*stream == '=') {
                        token.kind = TOKEN_GTEQ;
                        ++stream;
                }
                break;
        CASE1('^', '=', TOKEN_XOR_ASSIGN);
        CASE1(':', '=', TOKEN_COLON_ASSIGN);
        CASE1('/', '=', TOKEN_DIV_ASSIGN);
        CASE1('*', '=', TOKEN_MUL_ASSIGN);
        CASE1('%', '=', TOKEN_MOD_ASSIGN);
        CASE2('-', '=', TOKEN_SUB_ASSIGN, '-', TOKEN_DEC);
        CASE2('+', '=', TOKEN_ADD_ASSIGN, '+', TOKEN_INC);
        CASE2('&', '=', TOKEN_AND_ASSIGN, '|', TOKEN_AND);
        CASE2('|', '=', TOKEN_OR_ASSIGN, '|', TOKEN_OR);
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
                printf("TOKEN INT: %" PRIu64 "\n", token.int_val);
                break;
        case TOKEN_FLOAT:
                printf("TOKEN FLOAT: %f\n", token.float_val);
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

void
lex_test(void)
{
        // Integer literal tests.
        init_stream("0 18446744073709551615 0xffffffffffffffff 042 0b1111");
        assert_token_int(0);
        assert_token_int(18446744073709551615ULL);
        assert_token_mod(TOKENMOD_HEX);
        assert_token_int(0xffffffffffffffffULL);
        assert_token_mod(TOKENMOD_OCT);
        assert_token_int(042);
        assert_token_int(0xf);
        assert_token_eof();

        // Float literal tests.
        init_stream("3.14 .123 42. 3e10");
        assert_token_float(3.14);
        assert_token_float(.123);
        assert_token_float(42.);
        assert_token_float(3e10);
        assert_token_eof();

        // Char literal tests.
        init_stream("'a' '\\n'");
        assert_token_int('a');
        assert_token_int('\n');
        assert_token_eof();

        // String literal tests.
        init_stream("\"foo\" \"a\\nb\"");
        assert_token_str("foo");
        assert_token_str("a\nb");
        assert_token_eof();

        // Operator tests.
        init_stream(": := + += - -= -- ++ < <= << <<= > >= >> >>= ^ ^= / /= "
                        "* *= % %=");
        assert_token(':');
        assert_token(TOKEN_COLON_ASSIGN);
        assert_token('+');
        assert_token(TOKEN_ADD_ASSIGN);
        assert_token('-');
        assert_token(TOKEN_SUB_ASSIGN);
        assert_token(TOKEN_DEC);
        assert_token(TOKEN_INC);
        assert_token('<');
        assert_token(TOKEN_LTEQ);
        assert_token(TOKEN_LSHIFT);
        assert_token(TOKEN_LSHIFT_ASSIGN);
        assert_token('>');
        assert_token(TOKEN_GTEQ);
        assert_token(TOKEN_RSHIFT);
        assert_token(TOKEN_RSHIFT_ASSIGN);
        assert_token('^');
        assert_token(TOKEN_XOR_ASSIGN);
        assert_token('/');
        assert_token(TOKEN_DIV_ASSIGN);
        assert_token('*');
        assert_token(TOKEN_MUL_ASSIGN);
        assert_token('%');
        assert_token(TOKEN_MOD_ASSIGN);
        assert_token_eof();

        // Misc tests.
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
