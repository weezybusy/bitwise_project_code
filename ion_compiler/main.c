#include "common.h"
#include "lex.h"

void
run_tests(void)
{
        //buf_test();
        //intern_test();
        lex_test();
}

int
main(int argc, char **argv)
{
        run_tests();
        return 0;
}
