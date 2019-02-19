// Based on the lecture notes of Professor Tuck, heavily modified by Vincent
// Zhao

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "svec.h"

svec* make_svec(int refOnly) {
  svec* sv = malloc(sizeof(svec));
  sv->size = 0;
  sv->cap = 4;
  sv->data = malloc(4 * sizeof(char*));
  memset(sv->data, 0, 4 * sizeof(char*));
  sv->refOnly = refOnly;
  return sv;
}

/**
 * @brief Frees a {@code svec}.
 */
void free_svec(svec* sv) {
  free_svec_data(sv);
  free(sv);
}

/**
 * @brief Frees the data associated with a {@code svec}.
 * 
 * The actual {@code svec} will remain available for use.
 */
void free_svec_data(svec* sv) {
  if (!sv->refOnly) {
    for (int ii = 0; ii < sv->size; ++ii) {
      if (sv->data[ii] != 0) {
        free(sv->data[ii]);
      }
    }
  }
  free(sv->data);
}

char* svec_get(svec* sv, int ii) {
  assert(ii >= 0 && ii < sv->size);
  return sv->data[ii];
}

void svec_put(svec* sv, int ii, char* item) {
  assert(ii >= 0 && ii < sv->size);
  sv->data[ii] = strdup(item);
}

void svec_push_back(svec* sv, char* item) {
  int ii = sv->size;

  if (ii >= sv->cap) {
    sv->cap *= 2;
    sv->data = (char**)realloc(sv->data, sv->cap * sizeof(char*));
  }

  sv->size = ii + 1;
  if (sv->refOnly) {
    sv->data[ii] = item;
  } else {
    svec_put(sv, ii, item);
  }
}

void svec_reverse(svec* sv) {
  for (long ii = 0; ii < sv->size / 2; ii++) {
    char* temp = sv->data[ii];
    long reverseIdx = sv->size - 1 - ii;
    sv->data[ii] = sv->data[reverseIdx];
    sv->data[reverseIdx] = temp;
  }
}

void clear_svec(svec* sv) {
  free_svec_data(sv);
  sv->size = 0;
  sv->cap = 4;
  sv->data = malloc(4 * sizeof(char*));
  memset(sv->data, 0, 4 * sizeof(char*));
}

/**
 * @brief Gets a vector with references to the strings in the source in the range specified.
 * 
 * @param sv    The source {@code svec} to copy references from.
 * @param dst   The destination vector to store the range of references in.
 * @param start The first string to reference.
 * @param end   The last string to reference.
 */
void sub_svec(svec* sv, svec* dst, int start, int end) {
  assert(start <= end && end < sv->size && start >= 0);
  clear_svec(dst);
  for (int ii = start; ii <= end; ii++) {
    svec_push_back(dst, sv->data[ii]);
  }
}

/**
 * @brief Appends the contents of one {@svec} to the end of another.
 * 
 * The {@svec} added is freed automatically.
 */
void append_svec(svec* sv, svec* to_add) {
  for (int ii = 0; ii < to_add->size; ii++) {
    svec_push_back(sv, to_add->data[ii]);
  }
  free_svec(to_add);
}