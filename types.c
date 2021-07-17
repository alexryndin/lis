#include "types.h"

Integer plus (Integer a, Integer b) {
    Integer ret;
    ret.value = a.value + b.value;
    return ret;
}
