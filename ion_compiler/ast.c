#include "ast.h"

Typespec *
typespec_alloc(TypespecKind kind)
{
        Typespec *t;
        t = xcalloc(1, sizeof(Typespec));
        t->kind = kind;
        return t;
}

Typespec *
typespec_name(const char *name)
{
        Typespec *t;
        t = typespec_alloc(TYPESPEC_NAME);
        t->name = name;
        return t;
}

Typespec *
typespec_ptr(Typespec *elem)
{
        Typespec *t;
        t = typespec_alloc(TYPESPEC_PTR);
        t->ptr.elem = elem;
        return t;
}

Typespec *
typespec_array(Typespec *elem, Expr *size)
{
        Typespec *t;
        t = typespec_alloc(TYPESPEC_ARRAY);
        t->array.elem = elem;
        t->array.size = size;
        return t;
}

Typespec *
typespec_func(Typespec **args, size_t num_args, Typespec *ret)
{
        Typespec *t;
        t = typespec_alloc(TYPESPEC_FUNC);
        t->func.args = args;
        t->func.num_args = num_args;
        t->func.ret = ret;
        return t;
}

Expr *
expr_alloc(ExprKind kind)
{
        Expr *e;
        e = xcalloc(1, sizeof(Expr));
        e->kind = kind;
        return e;
}

Expr *
expr_int(uint64_t int_val)
{
        Expr *e;
        e = expr_alloc(EXPR_INT);
        e->int_val = int_val;
        return e;
}

Expr *
expr_float(double float_val)
{
        Expr *e;
        e = expr_alloc(EXPR_FLOAT);
        e->float_val = float_val;
        return e;
}

Expr *
expr_str(const char *str_val)
{
        Expr *e;

        e = expr_alloc(EXPR_STR);
        e->str_val = str_val;
        return e;
}

Expr *
expr_name(const char *name)
{
        Expr *e;
        e = expr_alloc(EXPR_NAME);
        e->name = name;
        return e;
}

Expr *
expr_cast(Typespec *type, Expr *expr)
{
        Expr *e;
        e = expr_alloc(EXPR_CAST);
        e->cast.type = type;
        e->cast.expr = expr;
        return e;
}

Expr *
expr_call(Expr *expr, Expr **args, size_t num_args)
{
        Expr *e;
        e = expr_alloc(EXPR_CALL);
        e->call.expr = expr;
        e->call.args = args;
        e->call.num_args = num_args;
        return e;
}

Expr *
expr_index(Expr *expr, Expr *index)
{
        Expr *e;
        e = expr_alloc(EXPR_INDEX);
        e->index.expr = expr;
        e->index.index = index;
        return e;
}

Expr *
expr_field(Expr *expr, const char *name)
{
        Expr *e;
        e = expr_alloc(EXPR_FIELD);
        e->field.expr = expr;
        e->field.name = name;
        return e;
}

Expr *
expr_unary(TokenKind op, Expr *expr)
{
        Expr *e;
        e = expr_alloc(EXPR_UNARY);
        e->unary.op = op;
        e->unary.expr = expr;
        return e;
}

Expr *
expr_binary(TokenKind op, Expr *left, Expr *right)
{
        Expr *e;
        e = expr_alloc(EXPR_BINARY);
        e->binary.op = op;
        e->binary.left = left;
        e->binary.right = right;
        return e;
}

Expr *
expr_ternary(Expr *cond, Expr *if_true, Expr *if_false)
{
        Expr *e;
        e = expr_alloc(EXPR_TERNARY);
        e->ternary.cond = cond;
        e->ternary.if_true = if_true;
        e->ternary.if_false = if_false;
        return e;
}

void
print_type(Typespec *type)
{
        Typespec *t;
        t = type;
        switch (t->kind) {
        case TYPESPEC_NAME:
                printf("%s", t->name);
                break;
        case TYPESPEC_FUNC: {
                printf("(func (");
                for (Typespec **it = t->func.args;
                                it != t->func.args + t->func.num_args; ++it) {
                       printf(" ");
                       print_type(*it);
                }
                printf(") ");
                print_type(t->func.ret);
                printf(")");
                break;
        }
        case TYPESPEC_ARRAY:
                printf("(arr ");
                print_type(t->array.elem);
                printf(" ");
                print_expr(t->array.size);
                printf(")");
                break;
        case TYPESPEC_PTR:
                printf("(ptr ");
                print_type(t->ptr.elem);
                printf(")");
                break;
        default:
                assert(0);
                break;
        }
}

void
print_expr(Expr *expr)
{
        Expr *e;
        e = expr;
        switch (e->kind) {
        case EXPR_INT:
                printf("%" PRIu64, e->int_val);
                break;
        case EXPR_FLOAT:
                printf("%f", e->float_val);
                break;
        case EXPR_STR:
                printf("\"%s\"", e->str_val);
                break;
        case EXPR_NAME:
                printf("%s", e->name);
                break;
        case EXPR_CAST:
                printf("(cast ");
                print_type(e->cast.type);
                printf(" ");
                print_expr(e->cast.expr);
                printf(")");
                break;
        case EXPR_CALL:
                printf("(");
                print_expr(e->call.expr);
                for (Expr **it = e->call.args;
                                it != e->call.args + e->call.num_args; ++it) {
                        printf(" ");
                        print_expr(*it);
                }
                printf(")");
                break;
        case EXPR_INDEX:
                printf("(index ");
                print_expr(e->index.expr);
                printf(" ");
                print_expr(e->index.index);
                printf(")");
                break;
        case EXPR_FIELD:
                printf("(field ");
                print_expr(e->field.expr);
                printf(" %s)", e->field.name);
                break;
        case EXPR_COMPOUND:
                printf("(compound ...)");
                break;
        case EXPR_UNARY:
                printf("(%c ", e->unary.op);
                print_expr(e->unary.expr);
                printf(")");
                break;
        case EXPR_BINARY:
                printf("(%c ", e->binary.op);
                print_expr(e->binary.left);
                printf(" ");
                print_expr(e->binary.right);
                printf(")");
                break;
        case EXPR_TERNARY:
                printf("(if ");
                print_expr(e->ternary.cond);
                printf(" ");
                print_expr(e->ternary.if_true);
                printf(" ");
                print_expr(e->ternary.if_false);
                printf(")");
                break;
        default:
                assert(0);
                break;
        }
}

void
expr_test(void)
{
        Expr *exprs[] = {
                expr_binary('+', expr_int(1), expr_int(2)),
                expr_unary('-', expr_float(3.14)),
                expr_ternary(expr_name("flag"), expr_str("true"),
                                expr_str("false")),
                expr_field(expr_name("person"), "name"),
                expr_call(expr_name("fact"), (Expr*[]) {expr_int(42)}, 1),
                expr_index(expr_field(expr_name("person"), "siblings"),
                                expr_int(3)),
                expr_cast(typespec_name("int_ptr"), expr_name("void_ptr"))
        };

        Expr **e;
        for (e = exprs; e != exprs + sizeof(exprs)/sizeof(*exprs); ++e) {
                print_expr(*e);
                printf("\n");
        }
}

void
ast_test(void)
{
        expr_test();
}
