#ifndef _AST_H_
#define _AST_H_

#include <stdint.h>

#include "common.h"
#include "lex.h"

typedef struct Expr Expr;
typedef struct Decl Decl;
typedef struct Stmt Stmt;
typedef struct FuncTypespec FuncTypespec;
typedef struct Typespec Typespec;

typedef enum TypespecKind {
        TYPESPEC_NONE,
        TYPESPEC_PAREN,
        TYPESPEC_NAME,
        TYPESPEC_FUNC,
        TYPESPEC_ARRAY,
        TYPESPEC_POINTER
} TypespecKind;

typedef struct FuncTypespec {
        BUF(Typespec **arg_types);
        Typespec *ret_type;
} FuncTypespec;

typedef struct Typespec {
        TypespecKind kind;
        struct {
                const char *name;
                Expr *index;
                FuncTypespec func;
        };
} Typespec;

typedef enum DeclKind {
        DECL_NONE,
        DECL_ENUM,
        DECL_STRUCT,
        DECL_UNION,
        DECL_VAR,
        DECL_CONST,
        DECL_TYPEDEF,
        DECL_FUNC
} DeclKind;

typedef struct EnumItem {
        const char *name;
        Typespec *type;
} EnumItem;

typedef struct AggregateItem {
        const char **names;
        Typespec *type;
} AggregateItem;

typedef struct FuncParam {
        const char *name;
        Typespec *type;
} FuncParam;

typedef struct FuncDecl {
        BUF(FuncParam *params); // BUF
        Typespec *ret_type;
} FuncDecl;

struct Decl {
        DeclKind kind;
        const char *name;
        union {
                BUF(EnumItem *enum_items); // BUF
                BUF(AggregateItem *aggregate_items); // BUF
                struct {
                        Typespec *type;
                        Expr *expr;
                };
                FuncDecl func_decl;
        };
};

typedef enum ExprKind {
        EXPR_NONE,
        EXPR_INT,
        EXPR_FLOAT,
        EXPR_STR,
        EXPR_NAME,
        EXPR_CAST,
        EXPR_CALL,
        EXPR_INDEX,
        EXPR_FIELD,
        EXPR_COMPOUND,
        EXPR_UNARY,
        EXPR_BINARY,
        EXPR_TERNARY
} ExprKind;

struct Expr {
        ExprKind kind;
        TokenKind op;

        union {
                // Literals.
                uint64_t int_val;
                double float_val;
                const char *str_val;
                const char *name;

                // Compound literals.
                struct {
                        Typespec *compound_type;
                        BUF(Expr **compound_args); // BUF
                };

                // Casts.
                struct {
                        Typespec *cast_type;
                        Expr *cast_expr;
                };

                // Unary.
                struct {
                        Expr *operand;
                        union {
                                BUF(Expr **args); // BUF
                                Expr *index;
                                const char *field;
                        };
                };

                // Binary.
                struct {
                        Expr *left;
                        Expr *right;
                };

                // Ternary.
                struct {
                        Expr *cond;
                        Expr *then_expr;
                        Expr *else_expr;
                };
        };
};

typedef enum StmtKind {
        STMT_NONE,
        STMT_RETURN,
        STMT_BREAK,
        STMT_CONTINUE,
        STMT_BLOCK,
        STMT_IF,
        STMT_WHILE,
        STMT_FOR,
        STMT_DO,
        STMT_SWITCH,
        STMT_ASSIGN,
        STMT_AUTO_ASSIGN,
        STMT_EXPR
} StmtKind;

typedef struct StmtBlock {
        BUF(Stmt **stmts);
} StmtBlock;

typedef struct ElseIf {
        Expr *cond;
        Stmt *block;
} ElseIf;

typedef struct Case {
        BUF(Expr **exprs);
        StmtBlock block;
} Case;

struct Stmt {
        StmtKind kind;
        Expr *expr;
        StmtBlock block;
        union {
                // If.
                struct {
                        BUF(ElseIf *elseifs);
                        StmtBlock else_block;
                };
                // For.
                struct {
                        StmtBlock for_init;
                        StmtBlock for_next;
                };
                // Switch.
                struct {
                        BUF(Case *cases);
                };
                // Auto-assign.
                struct {
                        const char *name;
                };
                // Assignment operators.
                struct {
                        Expr *rhs;
                };
        };
};

Expr *
expr_alloc(ExprKind kind);

Expr *
expr_int(uint64_t int_val);

Expr *
expr_float(double float_val);

Expr *
expr_str(const char *str_val);

Expr *
expr_name(const char *name);

Expr *
expr_cast(Typespec *type, Expr *expr);

Expr *
expr_unary(TokenKind op, Expr *expr);

Expr *
expr_binary(TokenKind op, Expr *left, Expr *right);

Expr *
expr_ternary(Expr *cond, Expr *then_expr, Expr *else_expr);

void
print_expr(Expr *expr);

void
expr_text(void);

void
ast_test(void);

#endif
