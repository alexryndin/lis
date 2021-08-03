#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "types.h"

LInteger new_l_integer(long v) {
    LInteger ret;
    ret.value = v;
    return ret;
}

void print_l_integer(LInteger *v) {
    printf("%li", v->value);
}

LValue *integer_plus (LValue *a, LValue *b) {
    LInteger ret;
    ret.value = a->value.l_int.value + b->value.l_int.value;
    return new_lval_int(ret);
}

LValue *integer_minus (LValue *a, LValue *b) {
    LInteger ret;
    ret.value = a->value.l_int.value - b->value.l_int.value;
    return new_lval_int(ret);
}

LValue *integer_mul (LValue *a, LValue *b) {
    LInteger ret;
    ret.value = a->value.l_int.value * b->value.l_int.value;
    return new_lval_int(ret);
}

LValue *integer_div (LValue *a, LValue *b) {
    if (b->value.l_int.value == 0) {
        return new_lval_err(LERR_DIV_ZERO, "");
    }
    LInteger ret;
    ret.value = a->value.l_int.value / b->value.l_int.value;
    return new_lval_int(ret);
}

LValue *integer_neg (LValue *a) {
    LInteger ret;
    ret.value = - a->value.l_int.value;
    return new_lval_int(ret);
}

LSym new_l_sym(int sym, char* repr) {
    LSym ret;
    ret.sym = sym;
    ret.repr = malloc(strlen(repr) + 1);
    strcpy(ret.repr, repr);
    return ret;
}


LErr new_l_err(int err, char* msg) {
    LErr ret;
    ret.en = err;
    ret.msg = malloc(strlen(msg) + 1);
    strcpy(ret.msg, msg);
    return ret;
}

LValue *new_lval_err(int err, char *msg) {
    LValue *ret = malloc(sizeof(LValue));
    ret->type = LVAL_ERR;
    ret->value.l_err = new_l_err(err, msg);
    return ret;
}

void print_l_err(LErr *v) {
    switch (v->en) {
    case LERR_DIV_ZERO: printf("Error: division by zero! "); break;
    case LERR_BAD_OP: printf("Error: invalid operator! "); break;
    case LERR_BAD_NUM: printf("Error: invalid number! "); break;
    case LERR_BAD_ARGS: printf("Error: invalid argument! "); break;
    case LERR_OTHER: break;
    }
    printf("%s", v->msg);
}

void print_l_sym(LSym *v) {
    printf("%s", v->repr);
}


LValue *new_lval_int(LInteger v) {
    LValue *ret = malloc(sizeof(LValue));
    ret->type = LVAL_INT;
    ret->value.l_int = v;
    return ret;
}

LValue *new_lval_sym(char *repr) {
    LValue *ret = malloc(sizeof(LValue));
    ret->type = LVAL_SYM;
    int sym;
    sym = 0;
    if (strcmp(repr, "+") == 0) sym = LPLUS;
    if (strcmp(repr, "-") == 0) sym = LMINUS;
    if (strcmp(repr, "*") == 0) sym = LMUL;
    if (strcmp(repr, "/") == 0) sym = LDIV;
    if (strcmp(repr, "head") == 0) sym = LHEAD;
    if (strcmp(repr, "tail") == 0) sym = LTAIL;
    if (strcmp(repr, "eval") == 0) sym = LEVAL;
    if (strcmp(repr, "list") == 0) sym = LLIST;
    if (strcmp(repr, "join") == 0) sym = LJOIN;
    ret->value.l_sym = new_l_sym(sym, repr);
    return ret;
}

LValue *new_lval_sexpr(void) {
    LValue *ret = malloc(sizeof(LValue));
    ret->type = LVAL_SEXPR;
    ret->value.lval = malloc(sizeof(LValue*));
    ret->len = 0;
    ret->size = 1;
    ret->start = 0;
    return ret;
}

LValue *new_lval_qexpr(void) {
    LValue *ret = malloc(sizeof(LValue));
    ret->type = LVAL_QEXPR;
    ret->value.lval = malloc(sizeof(LValue*));
    ret->len = 0;
    ret->size = 1;
    ret->start = 0;
    return ret;
}

void lval_del(LValue *v) {
    switch (v->type) {
        case LVAL_INT: break;
        case LVAL_ERR: free(v->value.l_err.msg); break;
        case LVAL_SYM: free(v->value.l_sym.repr); break;
        case LVAL_SEXPR: 
        case LVAL_QEXPR: 
            for (int i = 0; i < v->len; i++){
                lval_del(v->value.lval[i]);
            }

        break;
    }
    free(v);
    v = NULL;
}

void lval_free(LValue *v) {
    // if (!v) return;
    switch (v->type) {
        case LVAL_INT: break;
        case LVAL_ERR: free(v->value.l_err.msg); break;
        case LVAL_SYM: free(v->value.l_sym.repr); break;
        case LVAL_SEXPR: break;
        case LVAL_QEXPR: break;
    }
    free(v);
    v = NULL;
}

LValue *lval_plus(LValue *a, LValue *b) {
    if (a->type == LVAL_INT && b->type == LVAL_INT) {
        return integer_plus(a, b);
    } else {
        return new_lval_err(LERR_BAD_ARGS, "");
    }
}

LValue *lval_minus(LValue *a, LValue *b) {
    if (a->type == LVAL_INT && b->type == LVAL_INT) {
        return integer_minus(a, b);
    } else {
        return new_lval_err(LERR_BAD_ARGS, "");
    }
}

LValue *lval_mul(LValue *a, LValue *b) {
    if (a->type == LVAL_INT && b->type == LVAL_INT) {
        return integer_mul(a, b);
    } else {
        return new_lval_err(LERR_BAD_ARGS, "");
    }
}

LValue *lval_div(LValue *a, LValue *b) {
    if (a->type == LVAL_INT && b->type == LVAL_INT) {
        return integer_div(a, b);
    } else {
        return new_lval_err(LERR_BAD_ARGS, "");
    }
}

LValue *lval_neg(LValue *a) {
    if (a->type == LVAL_INT) {
        return integer_neg(a);
    } else {
        return new_lval_err(LERR_BAD_ARGS, "");
    }
}
