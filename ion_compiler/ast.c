#include "ast.h"

Expr *
expr_alloc(ExprKind kind)
{
        Expr *expr;

        expr = xcalloc(1, sizeof(Expr));
        expr->kind = kind;

        return expr;
}

Expr *
expr_int(uint64_t int_val)
{
        Expr *new_expr;

        new_expr = expr_alloc(EXPR_INT);
        new_expr->int_val = int_val;

        return new_expr;
}

Expr *
expr_float(double float_val)
{
        Expr *expr;

        expr = expr_alloc(EXPR_FLOAT);
        expr->float_val = float_val;

        return expr;
}

Expr *
expr_str(const char *str_val)
{
        Expr *expr;

        expr = expr_alloc(EXPR_STR);
        expr->str_val = str_val;

        return expr;
}

Expr *
expr_name(const char *name)
{
        Expr *expr;

        expr = expr_alloc(EXPR_NAME);
        expr->name = name;

        return expr;
}

Expr *
expr_cast(Typespec *type, Expr *expr)
{
        Expr *new_expr;

        new_expr = expr_alloc(EXPR_CAST);
        new_expr->cast_type = type;
        new_expr->cast_expr = expr;

        return new_expr;
}

Expr *
expr_unary(TokenKind op, Expr *expr)
{
        Expr *new_expr;

        new_expr = expr_alloc(EXPR_UNARY);
        new_expr->op = op;
        new_expr->operand = expr;

        return new_expr;
}

Expr *
expr_binary(TokenKind op, Expr *left, Expr *right)
{
        Expr *new_expr;

        new_expr = expr_alloc(EXPR_BINARY);
        new_expr->op = op;
        new_expr->left = left;
        new_expr->right = right;

        return new_expr;
}

Expr *
expr_ternary(Expr *cond, Expr *then_expr, Expr *else_expr)
{
        Expr *new_expr;

        new_expr = expr_alloc(EXPR_TERNARY);
        new_expr->cond = cond;
        new_expr->then_expr = then_expr;
        new_expr->else_expr = else_expr;

        return new_expr;
}

void
print_expr(Expr *expr)
{
        switch (expr->kind) {
        case EXPR_NONE:
                assert(0);
                break;
        case EXPR_INT:
                printf("%" PRIu64, expr->int_val);
                break;
        case EXPR_FLOAT:
                printf("%f", expr->float_val);
                break;
        case EXPR_STR:
                printf("\"%s\"", expr->str_val);
                break;
        case EXPR_NAME:
                printf("%s", expr->name);
                break;
        case EXPR_CAST:
                printf("(cast ");
                //print_type(expr->cast_type);
                printf(" ");
                print_expr(expr->cast_expr);
                printf(")");
                break;
        case EXPR_CALL:
                printf("(");
                print_expr(expr->operand);
                for (Expr **it = expr->args; it != buf_end(expr->args); ++it) {
                        printf(" ");
                        print_expr(*it);
                }
                printf(")");
                break;
        case EXPR_INDEX:
                printf("(index ");
                print_expr(expr->operand);
                printf(" ");
                print_expr(expr->index);
                printf(")");
                break;
        case EXPR_FIELD:
                printf("(field ");
                print_expr(expr->operand);
                printf(" %s", expr->field);
                break;
        case EXPR_COMPOUND:
                printf("(compound ...)");
                break;
        case EXPR_UNARY:
                printf("(%c ", expr->op);
                print_expr(expr->operand);
                printf(")");
                break;
        case EXPR_BINARY:
                printf("(%c ", expr->op);
                print_expr(expr->left);
                printf(" ");
                print_expr(expr->right);
                printf(")");
                break;
        case EXPR_TERNARY:
                printf("(if ");
                print_expr(expr->cond);
                printf(" ");
                print_expr(expr->then_expr);
                printf(" ");
                print_expr(expr->else_expr);
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
        Expr *expr;

        //expr = expr_int(42);
        //assert(expr->kind == EXPR_INT);
        //assert(expr->int_val == 42);
        expr = expr_binary('+', expr_int(1), expr_int(2));
        print_expr(expr);
}

void
ast_test(void)
{
        expr_test();
}
