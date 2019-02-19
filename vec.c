#include <stdlib.h>
#include <assert.h>
#include <stdio.h>

#include "vec.h"

vec* make_vec() {
    vec* v = malloc(sizeof(vec));
    v->size = 0;
    v->cap  = 4;
    v->data = malloc(4 * sizeof(int));
    return v;
}

void free_vec(vec* v) {
  free(v->data);
  free(v);
}

int vec_get(vec* v, int ii) {
    assert(ii >= 0 && ii < v->size);
    return v->data[ii];
}

void vec_push_back(vec* v, int item) {
    int ii = v->size;

    if (ii >= v->cap) {
        v->cap *= 2;
        v->data = (int*) realloc(v->data, v->cap * sizeof(int));
    }

    v->size = ii + 1;
    v->data[ii] = item;
}