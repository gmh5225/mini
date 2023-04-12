#include "minigen/errors.h"

MinigenError mg_errno = MG_ERR_NONE;

static const char *mg_error_table[] = {
    [MG_ERR_NONE] = "",
    [MG_ERR_INVALID_TARGET] = "invalid target for code generation",
    [MG_ERR_CHECK_ERRNO] = "check libc errno",
};

const char *minigen_error_str() {
    return mg_error_table[mg_errno];
}
