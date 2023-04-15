#include "util/vector.h"

#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>

#define VECTOR_DEFAULT_CAPACITY    16
#define VECTOR_DEFAULT_GROWTH_RATE 2

static bool vector_is_full(Vector *v) {
    return v->size == v->capacity;
}

static void vector_resize(Vector *v, size_t new_capacity) {
    assert(new_capacity > v->capacity);
    v->capacity = new_capacity;
    void *tmp = realloc(v->data, v->elem_size * v->capacity);
    v->data = tmp;
}

void vector_init(Vector *v, size_t elem_size) {
    v->elem_size = elem_size;
    v->size = 0;
    v->capacity = VECTOR_DEFAULT_CAPACITY;
    v->data = malloc(v->elem_size * v->capacity);
}

void vector_push_back(Vector *v, void *value) {
    if (vector_is_full(v))
        vector_resize(v, v->capacity * VECTOR_DEFAULT_GROWTH_RATE);
    v->data[v->size++] = value;
}

void *vector_get(Vector *v, size_t index) {
    if (index >= 0 && index < v->size)
        return v->data[index];
    return NULL;
}

void vector_clear(Vector *v) {
    for (size_t i = 0; i < v->size; i++) {
        v->data[i] = NULL;
    }
    v->size = 0;
}

void vector_free(Vector *v) {
    vector_clear(v);
    if (v->data) free(v->data);
}
