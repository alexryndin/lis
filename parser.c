#include <stdio.h>
#include <stdlib.h>

#include <readline/readline.h>
#include <readline/history.h>

#include "mpc/mpc.h"

#include "types.h"


Integer eval_op(char *op, Integer a, Integer b) {
    if (strcmp(op, "+") == 0) { Integer ret; ret.value = a.value + b.value; return ret; }
    if (strcmp(op, "-") == 0) { Integer ret; ret.value = a.value - b.value; return ret; }
    if (strcmp(op, "*") == 0) { Integer ret; ret.value = a.value * b.value; return ret; }
    if (strcmp(op, "/") == 0) { Integer ret; ret.value = a.value / b.value; return ret; }
    Integer ret;
    ret.value = -1;
    return ret;
}

Integer eval(mpc_ast_t* t) {
    if (strstr(t->tag, "num")) {
        Integer ret;
        ret.value = atoi(t->contents);
        return ret;
    }

    char *op = t->children[1]->contents;

    Integer x = eval(t->children[2]);
    for (int i = 3; strstr(t->children[i]->tag, "expr"); i++) {
        x = eval_op(op, x, eval(t->children[i]));
    }

    return x;
}


int main(int argc, char** argv) {

    mpc_parser_t *Number     = mpc_new("num");
    mpc_parser_t *Operator   = mpc_new("op");
    mpc_parser_t *Expression = mpc_new("expr");
    mpc_parser_t *L          = mpc_new("l");

    mpca_lang(MPCA_LANG_DEFAULT,
      "                                                \
        num  : /-?[0-9]+/ ;                            \
        op   : '+' | '-' | '*' | '/' ;                 \
        expr : <num> | '(' <op> <expr>+ ')' ;          \
        l    : /^/ <op> <expr>+ /$/ ;                  \
      ",
      Number, Operator, Expression, L);

    puts("lis version 0.2.0\n");
    puts("press ctrl+c to exit\n");

    while(1) {
        char* input = readline("lis> ");
        add_history(input);
        printf("you entered: %s\n", input);
        if ( ! input ) {
            printf("exiting\n");
            return 0;
        }
        mpc_result_t r;
        if (mpc_parse("<stdin>", input, L, &r)) {
            Integer res = eval(r.output);
            printf("%li\n", res.value);
            mpc_ast_delete(r.output);
        } else {
            mpc_err_print(r.error);
            mpc_err_delete(r.error);
        }
        free(input);
    }

    mpc_cleanup(4, Number, Operator, Expression, L);

    return 0;
}
