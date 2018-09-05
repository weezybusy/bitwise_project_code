#ifndef _AST_H_
#define _AST_H_

#include <stdint.h>

#include "common.h"
#include "lex.h"

typedef struct Expr Expr;
typedef struct Stmt Stmt;
typedef struct Decl Decl;
typedef struct Typespec Typespec;

typedef enum TypespecKind {
        TYPESPEC_NONE,
        TYPESPEC_NAME,
        TYPESPEC_FUNC,
        TYPESPEC_ARRAY,
        TYPESPEC_PTR
} TypespecKind;

typedef struct FuncTypespec {
        size_t num_args;
        Typespec **args;
        Typespec *ret;
} FuncTypespec;

typedef struct PtrTypespec {
        Typespec *elem;
} PtrTypespec;

typedef struct ArrayTypespec {
        Typespec *elem;
        Expr *size;
} ArrayTypespec;

struct Typespec {
        TypespecKind kind;
        union {
                const char *name;
                FuncTypespec func;
                ArrayTypespec array;
                PtrTypespec ptr;
        };
};

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

typedef struct FuncParam {
        const char *name;
        Typespec *type;
} FuncParam;

typedef struct FuncDecl {
        FuncParam *params;
        size_t num_params;
        Typespec *ret_type;
} FuncDecl;

typedef struct EnumItem {
        const char *name;
        Typespec *type;
} EnumItem;

typedef struct EnumDecl {
        const char *name;
        Typespec *type;
} EnumDecl;

typedef struct AggregateItem {
        const char **names;
        Typespec *type;
} AggregateItem;

typedef struct AggregateDecl {
        AggregateItem *items;
        size_t num_items;
} AggregateDecl;

typedef struct TypedefDecl {
        Typespec *type;
} TypedefDecl;

typedef struct VarDecl {
        Typespec *type;
        Expr *expr;
} VarDecl;

typedef struct ConstDecl {
        Expr *expr;
} ConstDecl;

struct Decl {
        DeclKind kind;
        const char *name;
        union {
                EnumDecl enum_decl;
                AggregateDecl aggregate;
                FuncDecl func;
                TypedefDecl typedef_decl;
                VarDecl var;
                ConstDecl const_decl;
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

typedef struct CompoundExpr {
        Typespec *type;
        Expr **args;
        size_t num_args;
} CompoundExpr;

typedef struct CastExpr {
        Typespec *type;
        Expr *expr;
} CastExpr;

typedef struct UnaryExpr {
        TokenKind op;
        Expr *expr;
} UnaryExpr;

typedef struct BinaryExpr {
        TokenKind op;
        Expr *left;
        Expr *right;
} BinaryExpr;

typedef struct TernaryExpr {
        Expr *cond;
        Expr *if_true;
        Expr *if_false;
} TernaryExpr;

typedef struct CallExpr {
        Expr *expr;
        Expr **args;
        size_t num_args;
} CallExpr;

typedef struct IndexExpr {
        Expr *expr;
        Expr *index;
} IndexExpr;

typedef struct FieldExpr {
        Expr *expr;
        const char *name;
} FieldExpr;

struct Expr {
        ExprKind kind;
        union {
                uint64_t int_val;
                double float_val;
                const char *str_val;
                const char *name;
                CompoundExpr compound;
                CastExpr cast;
                UnaryExpr unary;
                BinaryExpr binary;
                TernaryExpr ternary;
                CallExpr call;
                IndexExpr index;
                FieldExpr field;
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
        Stmt **stmts;
        size_t num_stmts;
} StmtBlock;

typedef struct ElseIf {
        Expr *cond;
        Stmt *block;
} ElseIf;

typedef struct IfStmt {
        Expr *cond;
        StmtBlock then_block;
        ElseIf *elseifs;
        size_t num_elseifs;
        StmtBlock else_block;
} IfStmt;

typedef struct WhileStmt {
        Expr *cond;
        StmtBlock block;
} WhileStmt;

typedef struct ForStmt {
        StmtBlock init;
        Expr *cond;
        StmtBlock next;
} ForStmt;

typedef struct SwitchCase {
        Expr **exprs;
        size_t num_exprs;
        StmtBlock block;
} SwitchCase;

typedef struct SwitchStmt {
        Expr *expr;
        SwitchCase *cases;
        size_t num_cases;
} SwitchStmt;

typedef struct AssignStmt {
        TokenKind op;
        Expr *left;
        Expr *right;
} AssignStmt;

typedef struct AutoAssignStmt {
        const char *name;
        Expr *init;
} AutoAssignStmt;

struct Stmt {
        StmtKind kind;
        union {
                IfStmt if_stmt;
                WhileStmt while_stmt;
                ForStmt for_stmt;
                SwitchStmt switch_stmt;
                AssignStmt assign;
                AutoAssignStmt autoassign;
        };
};

Typespec *
typespec_alloc(TypespecKind kind);

Typespec *
typespec_name(const char *name);

Typespec *
Typespec_ptr(Typespec *elem);

Typespec *
Typespec_array(Typespec *elem, Expr *size);

Typespec *
Typespec_func(Typespec **args, size_t num_args, Typespec *ret);

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
expr_call(Expr *expr, Expr **args, size_t num_args);

Expr *
expr_index(Expr *expr, Expr *index);

Expr *
expr_field(Expr *expr, const char *name);

Expr *
expr_unary(TokenKind op, Expr *expr);

Expr *
expr_binary(TokenKind op, Expr *left, Expr *right);

Expr *
expr_ternary(Expr *cond, Expr *if_true, Expr *if_false);

void
print_type(Typespec *type);

void
print_expr(Expr *expr);

void
expr_test(void);

void
ast_test(void);

#endif
