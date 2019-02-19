#ifndef VEC_H
#define VEC_H

// A vector of integers
typedef struct vec {
  int size;
  int cap;
  int* data;
} vec;

vec* make_vec();

void free_vec(vec* v);

int vec_get(vec* v, int ii);

void vec_push_back(vec* v, int item);

#endif
