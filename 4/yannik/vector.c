#include "vector.h"
#include "util.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

struct vector {
    int *data_;
    int capacity_;
    int top_index_;
};

static void vector_grow(vector *vector) {
    int new_capacity = vector->capacity_ * 2;
    int *new_data = realloc(vector->data_, new_capacity * sizeof(int));

    abort_on_failed_allocation(new_data);

    vector->data_ = new_data;
    vector->capacity_ = new_capacity;
}

vector *vector_create() {
    vector *v = (vector *)malloc(sizeof(vector));
    v->capacity_ = 2;
    v->top_index_ = -1;
    v->data_ = (int *)calloc(v->capacity_, sizeof(int));

    abort_on_failed_allocation(v->data_);

    return v;
}

void vector_free(vector *vector) {
    free(vector->data_);
    free(vector);
}

void vector_push_back(vector *vector, int number) {
    if (vector->top_index_ == vector->capacity_ - 1)
        vector_grow(vector);

    ++vector->top_index_;
    vector->data_[vector->top_index_] = number;
}

int *vector_data(vector *vector) {
    return vector->data_;
}

int vector_size(const vector *vector) {
    return vector->top_index_ + 1;
}

void vector_clear(vector *vector) {
    vector->top_index_ = -1;
}