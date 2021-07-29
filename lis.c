#include <stdio.h>
#include <stdlib.h>

#include <readline/readline.h>
#include <readline/history.h>

#include "mpc/mpc.h"

#include "types.h"


LValue *get_lval(LValue *v, int i);

LValue *builtin_op(LValue *op, LValue *a, LValue *b) {
    switch(op->value.l_sym.sym) {
        case LPLUS: return lval_plus(a, b); break;
        case LMINUS: return lval_minus(a, b); break;
        case LMUL: return lval_mul(a, b); break;
        case LDIV: return lval_div(a, b); break;
        default: return new_lval_err(LERR_BAD_OP, op->value.l_sym.repr);
    }
}


LValue *eval_op(LValue *op, LValue *lvals) {
    int count = lvals->len - lvals->start;
    if (op->value.l_sym.sym == LMINUS && count == 1) {
        return lval_neg(lvals->value.lval[lvals->start]);
    }
    LValue *first_arg = get_lval(lvals, 0);
    LValue *ret = builtin_op(op, first_arg, get_lval(lvals, 0));
    if (ret->type == LVAL_ERR) {
        return ret;
    }
    for (int i=lvals->start; i<lvals->len; i++) {
        LValue *res = builtin_op(op, ret, get_lval(lvals, 0));
        del_lval(ret);
        if (res->type == LVAL_ERR) {
            return res;
        }
        ret = res;
        res = NULL;
    }
    return ret;
}

LValue *lval_read_num(mpc_ast_t *t) {
    errno = 0;
    long x = strtol(t->contents, NULL, 10);
    return errno == ERANGE ? new_lval_err(LERR_BAD_NUM, t->contents)
                           : new_lval_int(new_l_integer(x));
}

LValue *lval_add(LValue *lvals, LValue *lval) {
    lvals->len++;
    if (lvals->len >= lvals->size) {
        lvals->size = lvals->size * 2;
        lvals->value.lval = realloc(lvals->value.lval, sizeof(LValue*) * lvals->size);
    }
    lvals->value.lval[lvals->len-1] = lval;

    return lvals;
}

LValue *lval_read(mpc_ast_t *t) {
    if (strstr(t->tag, "num")) return lval_read_num(t);
    if (strstr(t->tag, "sym")) return new_lval_sym(t->contents);

    LValue *lval = NULL;
    if (strcmp(t->tag, ">") == 0) lval = new_lval_sexpr();
    if (strstr(t->tag, "sexpr")) lval = new_lval_sexpr();
    for (int i = 0; i < t->children_num; i++) {
        if (strcmp(t->children[i]->contents, "(") == 0) continue;
        if (strcmp(t->children[i]->contents, ")") == 0) continue;
        // TODO: ???
        if (strcmp(t->children[i]->tag, "regex") == 0) continue;
        lval = lval_add(lval, lval_read(t->children[i]));
    }
    return lval;
}


//LValue *eval(mpc_ast_t* t) {
//    if (strstr(t->tag, "num")) {
//        errno = 0;
//        long x = strtol(t->contents, NULL, 10);
//        return errno == ERANGE ? new_lval_err(LERR_BAD_NUM)
//                               : new_lval_int(new_l_integer(x));
//    }
//
//    char *op = t->children[1]->contents;
//
//    LValue *x = eval(t->children[2]);
//    for (int i = 3; strstr(t->children[i]->tag, "expr"); i++) {
//        x = eval_op(op, x, eval(t->children[i]));
//    }
//
//    return x;
//}

LValue *take_lval(LValue *v, int idx) {
    for (int i = 0; i < idx; i++){
        del_lval(v->value.lval[i]);
    }
    for (int i = idx+1; i < v->len; i++){
        del_lval(v->value.lval[i]);
    }
    LValue *ret = v->value.lval[idx];
    free(v);
    return ret;
}

LValue *get_lval(LValue *v, int i) {
    if (i == 0) {
        return v->value.lval[v->start++];
    }

    exit(2);
}

LValue *eval_sexpr(LValue *v);

LValue *eval_lval(LValue *v) {
    if (v->type == LVAL_SEXPR) return eval_sexpr(v);
    return v;
}

LValue *eval_sexpr(LValue *v) {
    for (int i=v->start; i<v->len; i++) {
        v->value.lval[i] = eval_lval(v->value.lval[i]);
        if (v->value.lval[i]->type == LVAL_ERR) {
            return take_lval(v, i);
        } 
    }
    if (v->len == 0) return v;
    if (v->len == 1) return take_lval(v, 0);
    
    LValue *f = get_lval(v, 0);
    if (f->type != LVAL_SYM) {
    //    del_lval(f);
        del_lval(v);
        return new_lval_err(LERR_OTHER, "Symbol not found. S-expr must start with symbol");
    }

    LValue *ret = eval_op(f, v);
  //  del_lval(f);
    del_lval(v);
    return ret;

}

void lval_print(LValue *v, int println);

void lval_expr_print(LValue *v, char open, char close) {
    putchar(open);
    for (int i = v->start; i < v->len; i++) {
        lval_print(v->value.lval[i], 0);

        if (i != (v->len-1)) {
            putchar(' ');
        }
    }
    putchar(close);
}


void lval_print(LValue *v, int println) {
    switch (v->type) {
        case LVAL_INT: print_l_integer(&v->value.l_int); break;
        case LVAL_ERR: print_l_err(&v->value.l_err); break;
        case LVAL_SYM: print_l_sym(&v->value.l_sym); break;
        case LVAL_SEXPR: lval_expr_print(v, '(', ')'); break;
    }
    if (println) putchar('\n');
}


int main(int argc, char** argv) {
    mpc_parser_t *Number      = mpc_new("num");
    mpc_parser_t *Symbol      = mpc_new("sym");
    mpc_parser_t *Expression  = mpc_new("expr");
    mpc_parser_t *SExpression = mpc_new("sexpr");
    mpc_parser_t *L           = mpc_new("l");

    mpca_lang(MPCA_LANG_DEFAULT,
      "                                                 \
        num   : /-?[0-9]+/ ;                            \
        sym   : '+' | '-' | '*' | '/' ;                 \
        sexpr : '(' <expr>+ ')' ;                       \
        expr  : <num> | <sym> | <sexpr> ;               \
        l     : /^/ <expr>+ /$/ ; \
      ",
      Number, Symbol, Expression, SExpression, L);

    puts("lis version 0.3.0\n");
    puts("press ctrl+c to exit\n");

    while(1) {
        char* input = readline("lis> ");
        add_history(input);
        printf("< ");
        if ( ! input ) {
            printf("exiting\n");
            exit(0);
        }
        mpc_result_t r;
        if (mpc_parse("<stdin>", input, L, &r)) {
            // mpc_ast_print(r.output);
            // LValue *res = eval(r.output);
            LValue *res = lval_read(r.output);
            lval_print(res, 1);
            res = eval_lval(res);
            lval_print(res, 1);
            del_lval(res);
            mpc_ast_delete(r.output);
        } else {
            mpc_err_print(r.error);
            mpc_err_delete(r.error);
        }
        free(input);
    }

    mpc_cleanup(5, Number, Symbol, Expression, SExpression, L);

    return 0;
}
