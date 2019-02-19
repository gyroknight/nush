#ifndef NUSH_H
#define NUSH_H

#include "svec.h"
#include "vec.h"
#include "cmd_queue.h"
#include "flags.h"

int execute(svec* cmd, int* input_fd);

void execute_tok(svec* tokens, flags* flgs, vec* bg_pids);

int execute_bg(svec* cmd, int* input_fd);

void execute_bg_tok(svec* tokens, flags* flgs, vec* bg_pids);

int execute_red(char op, svec* cmd, char* file, int* input_fd);

void execute_red_tok(char op, svec* tokens, char* file, flags* flgs, vec* bg_pids);

int execute_pipe(svec* cmd, int* ret, int* input_fd);

void execute_pipe_tok(svec* tokens, flags* flgs, vec* bg_pids);

int execute_tokens(svec* tokens, flags* flgs, svec* buffer, vec* bg_pids, int bg_mode);

int next_command(svec* tokens, int current_idx);

void change_input(int* input_fd);

int check_bg(vec* bg_pids);

#endif
