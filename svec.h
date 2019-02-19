
// Based on the lecture notes of Professor Tuck, heavily modified by Vincent
// Zhao

#ifndef SVEC_H
#define SVEC_H

typedef struct svec {
  int size;
  int cap;
  char** data;
  int refOnly;
} svec;

svec* make_svec(int refOnly);
void free_svec(svec* sv);
void free_svec_data(svec* sv);

char* svec_get(svec* sv, int ii);
void svec_put(svec* sv, int ii, char* item);

void svec_push_back(svec* sv, char* item);

void svec_sort(svec* sv);

void svec_reverse(svec* sv);

void clear_svec(svec* sv);

void sub_svec(svec* sv, svec* dst, int start, int end);

void append_svec(svec* sv, svec* to_add);

#endif
