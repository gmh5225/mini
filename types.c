#include "mini.h"

const Type primitive_types[] = {
    [TYPE_VOID] = {
        .kind = TYPE_VOID,
        .name = "void",
        .align = 0,
        .size = 0,
    },
    [TYPE_INT] = {
        .kind = TYPE_INT,
        .name = "int",
        .align = 0,
        .size = sizeof(intmax_t),
    },
    [TYPE_UINT] = {
        .kind = TYPE_UINT,
        .name = "uint",
        .align = 0,
        .size = sizeof(uintmax_t),
    },
    [TYPE_FLOAT] = {
        .kind = TYPE_FLOAT,
        .name = "float",
        .align = 0,
        .size = sizeof(float),
    },
    [TYPE_DOUBLE] = {
        .kind = TYPE_DOUBLE,
        .name = "double",
        .align = 0,
        .size = sizeof(double),
    },
    [TYPE_CHAR] = {
        .kind = TYPE_CHAR,
        .name = "char",
        .align = 0,
        .size = sizeof(char),
    },
    [TYPE_BOOL] = {
        .kind = TYPE_BOOL,
        .name = "bool",
        .align = 0,
        .size = sizeof(bool),
    },
};
