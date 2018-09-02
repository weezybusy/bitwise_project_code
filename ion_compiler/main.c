#include "ast.h"
#include "common.h"
#include "lex.h"

void
run_tests(void)
{
        common_test();
        lex_test();
        ast_test();
}

int
main(int argc, char **argv)
{
        run_tests();
        return 0;
}
