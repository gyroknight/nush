#include <stdlib.h>

#include "flags.h"

flags* make_flags() {
    flags* f = malloc(sizeof(flags));
    f->ret = 0;
    f->are_tokens = 0;
    f->pipe_fd = 0;

    return f;
}