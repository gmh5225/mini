#ifndef MINIGEN_ERRORS_H
#define MINIGEN_ERRORS_H

typedef enum minigen_error_code {
    MG_ERR_NONE = 0,
    MG_ERR_INVALID_TARGET,
    MG_ERR_CHECK_ERRNO, // Errors that occur in libc (invalid fopen call, couldn't malloc, etc.)
} MinigenError;

extern MinigenError mg_errno;

const char *minigen_error_str();

#endif
