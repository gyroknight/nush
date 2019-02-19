#include <stdlib.h>
#include <assert.h>
#include <stdio.h>

#include "cmd_queue.h"
#include "svec.h"

cqueue* make_cqueue() {
    cqueue* cq = malloc(sizeof(cqueue));
    cq->size = 0;
    cq->cap  = 4;
    cq->queue = malloc(4 * sizeof(svec*));
    return cq;
}

void free_cqueue(cqueue* cq) {
  for(int ii = 0; ii < cq->size; ii++) {
    free_svec(cq->queue[ii]);
  }

  free(cq->queue);
  free(cq);
}

svec* cqueue_pop(cqueue* cq)
{
    assert(cq->size > 0);
    svec* cmd = cq->queue[--cq->size];
    return cmd;
}

void cqueue_push_back(cqueue* cq, svec* cmd)
{
    int ii = cq->size;

    if (ii >= cq->cap) {
        cq->cap *= 2;
        cq->queue = (svec**) realloc(cq->queue, cq->cap * sizeof(svec*));
    }

    cq->size = ii + 1;
    cq->queue[ii] = cmd;
}