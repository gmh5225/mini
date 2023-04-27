#ifndef MINI_TYPES_H
#define MINI_TYPES_H

#include <stdbool.h>

typedef enum TypeKind TypeKind;
typedef int TypeID;
typedef struct Type Type;

enum TypeKind
{
    TYPE_UNKNOWN,
    TYPE_VOID, // Primitive types
    TYPE_INT,
    TYPE_UINT,
    TYPE_FLOAT,
    TYPE_DOUBLE,
    TYPE_CHAR,
    TYPE_BOOL,
    TYPE_STRUCT, // User-defined
    TYPE_ENUM,
};

struct Type
{
    TypeKind kind;
    TypeID id;
    char *name;
    int align;
    int size;
    bool is_pointer;
};

extern const Type primitive_types[];

#endif
