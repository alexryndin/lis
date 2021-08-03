#include <stdio.h>
#include <stdlib.h>

#include <readline/readline.h>
#include <readline/history.h>

#include "mpc/mpc.h"

#include "types.h"


#define LASSERT(args, cond, err, msg) \
    if (!(cond)) { lval_del(args); return new_lval_err(err, msg); }



LValue *lval_get(LValue *v, int i);
LValue *lval_take(LValue *v, int idx);
LValue *lval_only(LValue *v, int idx);
LValue *lval_eval(LValue *v);
LValue *lval_add(LValue *lvals, LValue *lval);
void lval_print(LValue *v, int println);
LValue *eval_op(LValue *op, LValue *lvals);
LValue *builtin_list(LValue *lvals);
LValue *builtin_head(LValue *lvals);
LValue *builtin_tail(LValue *lvals);
LValue *builtin_join(LValue *lvals);
LValue *builtin_eval(LValue *lvals);

LValue *builtin(LValue *op, LValue *lvals) {
    switch (op->value.l_sym.sym) {
        case LPLUS:
        case LMINUS:
        case LMUL:
        case LDIV:
            return eval_op(op, lvals);
        // Only eval_op uses op, delete it here;
        lval_del(op);
        case LLIST:
            return builtin_list(lvals);
        case LHEAD:
            return builtin_head(lvals);
        case LTAIL:
            return builtin_tail(lvals);
        case LJOIN:
            return builtin_join(lvals);
        case LEVAL:
            return builtin_eval(lvals);
    }
}

LValue *_eval_op(LValue *op, LValue *a, LValue *b) {
    switch(op->value.l_sym.sym) {
        case LPLUS: return lval_plus(a, b); break;
        case LMINUS: return lval_minus(a, b); break;
        case LMUL: return lval_mul(a, b); break;
        case LDIV: return lval_div(a, b); break;
        default: return new_lval_err(LERR_BAD_OP, op->value.l_sym.repr);
    }
}

// Cleanup
LValue *builtin_head(LValue *lvals) {
    int count = lvals->len - lvals->start;
    LASSERT(lvals, count == 1, LERR_OTHER, "Function 'head' passed too many arguments");

    LASSERT(lvals, lvals->value.lval[lvals->start]->type == LVAL_QEXPR, LERR_OTHER, "Function 'head' passed incorrect types")

    int child_count = lvals->value.lval[lvals->start]->len   -
                      lvals->value.lval[lvals->start]->start  ;
    LASSERT(lvals, child_count != 0, LERR_OTHER, "Function 'head' passed empty q-expr")

    LValue *ret = lval_take(lvals, 0); // ( '(...) ) -> '(...)
    return lval_only(ret, ret->start); // '( x1, x2 ... xn ) -> '(x1)
}


// Doesn't cleanup
LValue *builtin_tail(LValue *lvals) {
    int count = lvals->len - lvals->start;
    LASSERT(lvals, count == 1, LERR_OTHER, "Function 'tail' passed too many arguments");

    LASSERT(lvals, lvals->value.lval[lvals->start]->type == LVAL_QEXPR, LERR_OTHER, "Function 'tail' passed incorrect types");

    int child_count = lvals->value.lval[lvals->start]->len   -
                      lvals->value.lval[lvals->start]->start  ;
    LASSERT(lvals, child_count != 0, LERR_OTHER, "Function 'tail' passed empty q-expr")

    LValue *ret = lval_take(lvals, 0); // ( '(...) ) -> '(...)
    lval_get(ret, 0); // increment
    return ret;
}


LValue *builtin_list(LValue *v) {
    v->type = LVAL_QEXPR;
    return v;
}

LValue *builtin_eval(LValue *lvals) {
    int count = lvals->len - lvals->start;
    LASSERT(lvals, count == 1, LERR_OTHER, "Function 'eval' passed too many arguments");
    LASSERT(lvals, lvals->value.lval[lvals->start]->type == LVAL_QEXPR, LERR_OTHER, "Function 'eval' passed incorrect types");

    LValue *ret = lval_take(lvals, 0); // ( '(...) ) -> '(...)
    ret->type = LVAL_SEXPR;
    return lval_eval(ret);
}

LValue *builtin_join(LValue *lvals) {
    for (int i=lvals->start; i<lvals->len; i++) {
        LASSERT(lvals, lvals->value.lval[i]->type == LVAL_QEXPR,
                LERR_OTHER, "Function 'join' passed incorrect type");
    }
    LValue *ret = lval_get(lvals, 0);
    while (lvals->start < lvals->len) {
         LValue *b = lval_get(lvals, 0);
         while (b->start < b->len) {
             lval_add(ret, lval_get(b, 0));
         }
         lval_free(b);

    }
    lval_free(lvals);
    return ret;
}

LValue *eval_op(LValue *op, LValue *lvals) {
    int count = lvals->len - lvals->start;
    if (op->value.l_sym.sym == LMINUS && count == 1) {
        return lval_neg(lvals->value.lval[lvals->start]);
    }
    LValue *first_arg = lval_get(lvals, 0);
    LValue *ret = _eval_op(op, first_arg, lval_get(lvals, 0));
    if (ret->type == LVAL_ERR) {
        return ret;
    }
    for (int i=lvals->start; i<lvals->len; i++) {
        LValue *res = _eval_op(op, ret, lval_get(lvals, 0));
        lval_del(ret);
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
    if (strstr(t->tag, "qexpr")) lval = new_lval_qexpr();
    for (int i = 0; i < t->children_num; i++) {
        if (strcmp(t->children[i]->contents, "'") == 0) continue;
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
//
LValue *lval_only(LValue *v, int idx){
    for (int i = 0; i < idx; i++){
        lval_del(v->value.lval[i]);
    }
    for (int i = idx+1; i < v->len; i++){
        lval_del(v->value.lval[i]);
    }
    LValue *only = v->value.lval[idx];
    v->len = 1;
    v->size = 1;
    v->start = 0;
    free(v->value.lval);
    v->value.lval = malloc(sizeof(LValue*));
    v->value.lval[0] = only;
    return v;
}

LValue *lval_take(LValue *v, int idx) {
    idx = idx + v->start;
    for (int i = 0; i < idx; i++){
        lval_del(v->value.lval[i]);
    }
    for (int i = idx+1; i < v->len; i++){
        lval_del(v->value.lval[i]);
    }
    LValue *ret = v->value.lval[idx];
    free(v);
    return ret;
}

LValue *lval_get(LValue *v, int i) {
    if (i == 0) {
        return v->value.lval[v->start++];
    }

    exit(2);
}

LValue *eval_sexpr(LValue *v);

LValue *lval_eval(LValue *v) {
    if (v->type == LVAL_SEXPR) return eval_sexpr(v);
    return v;
}

LValue *eval_sexpr(LValue *v) {
    for (int i=v->start; i<v->len; i++) {
        v->value.lval[i] = lval_eval(v->value.lval[i]);
        if (v->value.lval[i]->type == LVAL_ERR) {
            return lval_take(v, i);
        } 
    }
    if (v->len == 0) return v;
    if (v->len == 1) return lval_take(v, 0);
    
    LValue *f = lval_get(v, 0);
    if (f->type != LVAL_SYM) {
    //    lval_del(f);
        lval_del(v);
        return new_lval_err(LERR_OTHER, "Symbol not found. S-expr must start with symbol");
    }

    LValue *ret = builtin(f, v);
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
        case LVAL_QEXPR: putchar('\'');
        case LVAL_SEXPR: lval_expr_print(v, '(', ')'); break;
    }
    if (println) putchar('\n');
}


int main(int argc, char** argv) {
    mpc_parser_t *Number      = mpc_new("num");
    mpc_parser_t *Symbol      = mpc_new("sym");
    mpc_parser_t *Expression  = mpc_new("expr");
    mpc_parser_t *SExpression = mpc_new("sexpr");
    mpc_parser_t *QExpression = mpc_new("qexpr");
    mpc_parser_t *L           = mpc_new("l");

    mpca_lang(MPCA_LANG_DEFAULT,
      "                                                 \
        num   : /-?[0-9]+/ ;                            \
        sym   : \"head\" | \"tail\" | \"join\" |        \
                \"list\" | \"eval\" |                   \
                '+' | '-' | '*' | '/' ;                 \
        sexpr : '(' <expr>+ ')' ;                       \
        qexpr : '\'' '(' <expr>+ ')' ;                  \
        expr  : <num> | <sym> | <sexpr> | <qexpr> ;     \
        l     : /^/ <expr>+ /$/ ; \
      ",
      Number, Symbol, Expression, QExpression, SExpression, L);

    puts("lis version 0.4.0\n");
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
            res = lval_eval(res);
            lval_print(res, 1);
            lval_del(res);
            mpc_ast_delete(r.output);
        } else {
            mpc_err_print(r.error);
            mpc_err_delete(r.error);
        }
        free(input);
    }

    mpc_cleanup(5, Number, Symbol, Expression, SExpression, QExpression, L);

    return 0;
}
