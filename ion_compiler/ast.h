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
        size_t num_args;
        Typespec **args;
        Typespec *ret;
} FuncTypespec;

typedef struct Typespec {
        TypespecKind kind;
        struct {
                const char *name;
                FuncTypespec func;
                struct {
                        Typespec *base;
                        Expr *size;
                };
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
        size_t names;
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
        FuncParam *params;
        size_t num_params;
        Typespec *ret_type;
} FuncDecl;

struct Decl {
        DeclKind kind;
        const char *name;
        union {
                struct {
                        size_t num_enum_items;
                        EnumItem *enum_items;
                };
                struct {
                        size_t num_aggregate_items;
                        AggregateItem *aggregate_items;
                };
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
                        size_t num_compound_args;
                        Typespec *compound_type;
                        Expr **compound_args;
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
                                struct {
                                        size_t num_args;
                                        Expr **args;
                                };
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
        size_t num_stmts;
        Stmt **stmts;
} StmtBlock;

typedef struct ElseIf {
        Expr *cond;
        Stmt *block;
} ElseIf;

typedef struct Case {
        size_t num_exprs;
        Expr **exprs;
        StmtBlock block;
} Case;

struct Stmt {
        StmtKind kind;
        Expr *expr;
        StmtBlock block;
        union {
                // If.
                struct {
                        size_t num_elseifs;
                        ElseIf *elseifs;
                        StmtBlock else_block;
                };
                // For.
                struct {
                        StmtBlock for_init;
                        StmtBlock for_next;
                };
                // Switch.
                struct {
                        size_t num_cases;
                        Case *cases;
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

Typespec *
typespec_alloc(TypespecKind kind);

Typespec *
typespec_name(const char *name);

Typespec *
Typespec_pointer(Typespec *base);

Typespec *
Typespec_array(Typespec *base, Expr *size);

Typespec *
Typespec_func(FuncTypespec func);

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
expr_call(Expr *operand, size_t num_args, Expr **args);

Expr *
expr_index(Expr *operand, Expr *index);

Expr *
expr_field(Expr *operand, const char *field);

Expr *
expr_unary(TokenKind op, Expr *expr);

Expr *
expr_binary(TokenKind op, Expr *left, Expr *right);

Expr *
expr_ternary(Expr *cond, Expr *then_expr, Expr *else_expr);

void
print_type(Typespec *type);

void
print_expr(Expr *expr);

void
print_expr_line(Expr *expr);

void
expr_test(void);

void
ast_test(void);

#endif
