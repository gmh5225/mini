#ifndef MINI_VECTOR_H 
#define MINI_VECTOR_H

#include <stddef.h>

typedef struct Vector Vector;
struct Vector
{
    size_t elem_size;
    size_t size;
    size_t capacity;
    void **data;
};

void vector_init(Vector *v, size_t elem_size);
void vector_push_back(Vector *v, void *value);
void *vector_get(Vector *v, size_t index);
void vector_clear(Vector *v);
void vector_free(Vector *v);

#endif
