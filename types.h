enum l_val { LVAL_SYM, LVAL_SEXPR, LVAL_QEXPR, LVAL_INT, LVAL_ERR };

enum l_err { LERR_DIV_ZERO, LERR_BAD_OP, LERR_BAD_NUM, LERR_BAD_ARGS, LERR_OTHER };

enum l_sym { LPLUS, LMINUS, LMUL, LDIV, LLIST, LHEAD,
    LTAIL, LJOIN, LEVAL };

typedef struct LInteger LInteger;
typedef struct LInteger {
    long value;
} LInteger;

typedef struct {
    char* msg;
    enum l_err en;
} LErr;

typedef struct {
    char* repr;
    enum l_sym sym;
} LSym;

typedef struct LValue {
    int type;
    union {
        LInteger l_int;
        LErr l_err;
        LSym l_sym;
        struct LValue **lval;
    } value;
    unsigned int len;
    unsigned int size;
    unsigned int start;
} LValue;

LInteger new_l_integer();
LValue *new_lval_err(int err, char *msg);
LValue *new_lval_int(LInteger v);
LValue *new_lval_sym(char *repr);
LValue *new_lval_sexpr();
LValue *new_lval_qexpr();

LValue *lval_neg (LValue *a);
LValue *lval_plus (LValue *a, LValue *b);
LValue *lval_minus (LValue *a, LValue *b);
LValue *lval_mul (LValue *a, LValue *b);
LValue *lval_div (LValue *a, LValue *b);
void lval_del(LValue *v);
void lval_free(LValue *v);

void print_l_integer(LInteger *v);
void print_l_err(LErr *v);
void print_l_sym(LSym *v);
