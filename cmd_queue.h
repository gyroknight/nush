#ifndef CMD_QUEUE_H
#define CMD_QUEUE_H

#include "svec.h"

/**
 * @brief A vector of {@code svec}s.
 * 
 * Can be used as either a queue for commands or to store a history of tokens.
 */
typedef struct cqueue {
  int size;
  int cap;
  svec** queue;
} cqueue;

cqueue* make_cqueue();

void free_cqueue(cqueue* cq);

svec* cqueue_pop(cqueue* cq);

void cqueue_push_back(cqueue* cq, svec* cmd);

#endif
