#include <assert.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "nush.h"
#include "tokens.h"
#include "vec.h"

/**
 * @brief Executes a command with its arguments.
 *
 * @param cmd is the command and arguments to run.
 * @param input_fd is a pipe to replace stdin with.
 * @return int the exit status of the command.
 */
int execute(svec* cmd, int* input_fd) {
  assert(cmd->size > 0);
  int cpid;
  int ret = 0;
  if (cpid = fork()) {
    if (*input_fd > 0) {
      close(*input_fd);
      *input_fd = 0;
    }
    int status;
    waitpid(cpid, &status, 0);
    ret = WEXITSTATUS(status);
  } else {
    change_input(input_fd);
    svec_push_back(cmd, 0);
    execvp(cmd->data[0], cmd->data);
  }

  clear_svec(cmd);

  return ret;
}

/**
 * @brief Executes a set of tokens.
 *
 * Changes stdin if a pipe is available.
 *
 * @param tokens is the set of tokens to execute.
 * @param flgs is the (current) flags to use.
 * @param bg_pids is the list of backgrounded processes to check later.
 */
void execute_tok(svec* tokens, flags* flgs, vec* bg_pids) {
  assert(tokens->size > 0);
  flgs->are_tokens = 0;
  svec* buffer = make_svec(1);
  int cpid = 1;
  int ret = 0;
  if (flgs->pipe_fd > 0 && (cpid = fork())) {
    close(flgs->pipe_fd);
    flgs->pipe_fd = 0;
    int status;
    waitpid(cpid, &status, 0);
    ret = WEXITSTATUS(status);
  } else {
    change_input(&flgs->pipe_fd);
    execute_tokens(tokens, flgs, buffer, bg_pids, 0);
    if (!cpid) {
      _exit(flgs->ret);
    }
  }
  free_svec(buffer);
  clear_svec(tokens);
  if (ret != 0) {
    flgs->ret = ret;
  }
}

/**
 * @brief Executes a command and its arguments in the background.
 *
 * @param cmd is the command to run.
 * @param input_fd is the input stream to use.
 * @return int is the PID of the brackgrounded process.
 */
int execute_bg(svec* cmd, int* input_fd) {
  assert(cmd->size > 0);
  int cpid;
  if (cpid = fork()) {
    if (*input_fd > 0) {
      close(*input_fd);
      *input_fd = 0;
    }
  } else {
    change_input(input_fd);
    svec_push_back(cmd, 0);
    execvp(cmd->data[0], cmd->data);
  }

  clear_svec(cmd);

  return cpid;
}

/**
 * @brief Executes a set of tokens in the background
 *
 * @param tokens
 * @param flgs
 * @param bg_pids
 */
void execute_bg_tok(svec* tokens, flags* flgs, vec* bg_pids) {
  assert(tokens->size > 0);
  flgs->are_tokens = 0;
  int cpid = 1;
  if (flgs->pipe_fd > 0 && (cpid = fork())) {
    close(flgs->pipe_fd);
    flgs->pipe_fd = 0;
    int status;
    waitpid(cpid, &status, 0);
    vec_push_back(bg_pids, WEXITSTATUS(status));
  } else {
    change_input(&flgs->pipe_fd);
    if (!cpid) {
      _exit(execute_tokens(tokens, NULL, NULL, NULL, 1));
    } else {
      vec_push_back(bg_pids, execute_tokens(tokens, NULL, NULL, NULL, 1));
    }
  }
  clear_svec(tokens);
}

/**
 * @brief Handles command execution with file redirection
 *
 * @param op        is the type of redirect, either < or >.
 * @param cmd       is the command to execute.
 * @param file      is the name of the file to use.
 * @param input_fd  is a stored pipe to use as stdin.
 * @return int      is the exit status of the command.
 */
int execute_red(char op, svec* cmd, char* file, int* input_fd) {
  assert(cmd->size > 0 != *input_fd > 0);
  int cpid;
  int ret = 0;
  if (cpid = fork()) {
    if (*input_fd > 0) {
      close(*input_fd);
      *input_fd = 0;
    }
    int status;
    waitpid(cpid, &status, 0);
    ret = WEXITSTATUS(status);
  } else {
    if (op == '<') {
      // If you're using a file as input, you shouldn't have a queued pipe input
      // as well.
      assert(*input_fd == 0);
      int inputfd = open(file, O_RDONLY, 0444);
      dup2(inputfd, 0);
      close(inputfd);
    } else {
      int outputfd = open(file, O_CREAT | O_WRONLY, 0644);
      dup2(outputfd, 1);
      close(outputfd);
    }

    if (*input_fd > 0) {
      change_input(input_fd);
      // I hope you don't have more than 4096 characters in your pipe.
      char strm_buf[4096];
      read(0, strm_buf, 4096);
      write(1, strm_buf, 4096);
      _exit(0);
    } else {
      svec_push_back(cmd, 0);
      execvp(cmd->data[0], cmd->data);
    }
  }

  clear_svec(cmd);

  return ret;
}

/**
 * @brief Handles executing a set of tokens with file redirection
 *
 * @param op      is the type of redirect, either < or >.
 * @param tokens  is the set of tokens to execute.
 * @param file    is the name of the file to use.
 * @param flgs    are the (current) flags to use.
 * @param bg_pids is the list of background process to check later.
 */
void execute_red_tok(char op, svec* tokens, char* file, flags* flgs,
                     vec* bg_pids) {
  assert(tokens->size > 0);
  flgs->are_tokens = 0;
  svec* buffer = make_svec(1);
  int cpid;
  int ret = 0;
  if (cpid = fork()) {
    if (flgs->pipe_fd > 0) {
      close(flgs->pipe_fd);
      flgs->pipe_fd = 0;
    }
    int status;
    waitpid(cpid, &status, 0);
    ret = WEXITSTATUS(status);
  } else {
    if (op == '<') {
      assert(flgs->pipe_fd == 0);
      int inputfd = open(file, O_RDONLY, 0444);
      dup2(inputfd, 0);
      close(inputfd);
    } else {
      int outputfd = open(file, O_CREAT | O_WRONLY, 0644);
      dup2(outputfd, 1);
      close(outputfd);
    }

    if (flgs->pipe_fd > 0) {
      change_input(&flgs->pipe_fd);
      char strm_buf[4096];
      read(0, strm_buf, 4096);
      write(1, strm_buf, 4096);
      _exit(0);
    } else {
      execute_tokens(tokens, flgs, buffer, bg_pids, 0);
      _exit(flgs->ret);
    }
  }

  free_svec(buffer);
  clear_svec(tokens);

  if (ret != 0) {
    flgs->ret = ret;
  }
}

/**
 * @brief Creates a pipe, executes a command, returns the pipe with the stored
 * output
 *
 * The write port of the pipe is automatically closed after usage.
 *
 * @param cmd       is the command to execute.
 * @param ret       is the (current) return value of the shell.
 * @param input_fd  is the input stream to use.
 * @return int      is the file descriptor of the pipe's read port.
 */
int execute_pipe(svec* cmd, int* ret, int* input_fd) {
  assert(cmd->size > 0);
  int pipe_fds[2];
  int rv = pipe(pipe_fds);
  assert(rv == 0);

  int cpid;

  if (cpid = fork()) {
    if (*input_fd > 0) {
      close(*input_fd);
      *input_fd = 0;
    }
    close(pipe_fds[1]);
    int status;
    waitpid(cpid, &status, 0);
    *ret = WEXITSTATUS(status);
    clear_svec(cmd);
  } else {
    dup2(pipe_fds[1], 1);
    close(pipe_fds[0]);
    close(pipe_fds[1]);
    _exit(execute(cmd, input_fd));
  }

  return pipe_fds[0];
}

/**
 * @brief Creates a pipe, executes a set of tokens, and sets the appropriate
 * flag
 *
 * @param tokens  is the tokens to execute.
 * @param flgs    are the (current) flags to use.
 * @param bg_pids is the list of background processes to check later.
 */
void execute_pipe_tok(svec* tokens, flags* flgs, vec* bg_pids) {
  assert(tokens->size > 0);
  flgs->are_tokens = 0;
  svec* buffer = make_svec(1);
  int pipe_fds[2];
  int rv = pipe(pipe_fds);
  assert(rv == 0);

  int cpid;

  if (cpid = fork()) {
    if (flgs->pipe_fd > 0) {
      close(flgs->pipe_fd);
      flgs->pipe_fd = 0;
    }
    close(pipe_fds[1]);
    int status;
    waitpid(cpid, &status, 0);
    flgs->ret = WEXITSTATUS(status);
    free_svec(buffer);
    clear_svec(tokens);
    flgs->pipe_fd = pipe_fds[0];
  } else {
    dup2(pipe_fds[1], 1);
    close(pipe_fds[0]);
    close(pipe_fds[1]);
    _exit(execute_tokens(tokens, flgs, buffer, bg_pids, 0));
  }
}

/**
 * @brief Returns the index of the token right before the next command to
 * process
 *
 * The index right before is returned so that when processed in a loop, the
 * correct token gets processed.
 *
 * @param tokens        is the tokens to execute.
 * @param current_idx   is the current token's index.
 * @return int          is the index of the token right before the next command.
 */
int next_command(svec* tokens, int current_idx) {
  int ii = current_idx;
  int current_lvl = 0;
  while (ii < tokens->size) {
    char* token = tokens->data[ii];
    // Takes you to the token right after the matching parenthesis
    if (strcmp(token, "(") == 0) {
      current_lvl++;
    } else if (strcmp(token, ")") == 0) {
      if (--current_lvl == 0) {
        ii++;
        break;
      }
    } else if (current_lvl == 0 &&
                   (strcmp(token, ";") == 0 || strcmp(token, "|") == 0 ||
                    strcmp(token, ">") == 0) ||
               strcmp(token, "&") == 0) {
      break;
    }
    ii++;
  }

  return ii - 1;
}

/**
 * @brief Replace stdin with the provided file descriptor.
 *
 * This should only be used in a fork. The provided file descriptor is closed.
 *
 * @param input_fd
 */
void change_input(int* input_fd) {
  if (*input_fd > 0) {
    dup2(*input_fd, 0);
    close(*input_fd);
    *input_fd = 0;
  }
}

/**
 * @brief Checks that all the provided processes have exited
 *
 * @param bg_pids is the processes to check.
 * @return int    is the exit code of the processes, the first non-zero code is
 * returned.
 */
int check_bg(vec* bg_pids) {
  int ret = 0;
  for (int ii = 0; ii < bg_pids->size; ii++) {
    int status;
    waitpid(bg_pids->data[ii], &status, 0);
    if (ret == 0) {
      ret = WEXITSTATUS(status);
    }
  }

  free_vec(bg_pids);

  return ret;
}

/**
 * @brief Executes a set of tokens.
 *
 * If background mode is set, {@code buffer}, {@code flgs}, and {@code bg_pids}
 * are ignored. New local copies are created.
 *
 * @param tokens  is the tokens to execute.
 * @param flgs    is the flags to use.
 * @param buffer  is the command buffer to use.
 * @param bg_pids is the list of background processes to check later.
 * @param bg_mode determines if the tokens should be run in the background or
 * not.
 * @return int    is 0 if not in the background, and the PID of the child if it
 * is.
 */
int execute_tokens(svec* tokens, flags* flgs, svec* buffer, vec* bg_pids,
                   int bg_mode) {
  // If the end token is &, background mode is enabled
  if (tokens->size > 0 && strcmp(tokens->data[tokens->size - 1], "&") == 0) {
    bg_mode = 1;
  }

  if (bg_mode) {
    buffer = make_svec(1);
    bg_pids = make_vec();
    flgs = make_flags();
  }

  int cpid;

  if (bg_mode && (cpid = fork())) {
    free_svec(buffer);
    free_vec(bg_pids);
    free(flgs);

    return cpid;
  } else {
    for (int ii = 0; ii < tokens->size; ii++) {
      char* token = tokens->data[ii];
      if (strcmp(token, ";") == 0) {
        // ; executes whatever has been accumulated in the buffer
        if (buffer->size > 0) {
          if (flgs->are_tokens) {
            execute_tok(buffer, flgs, bg_pids);
          } else {
            flgs->ret = execute(buffer, &flgs->pipe_fd);
          }
        }
      } else if (strcmp(token, "||") == 0) {
        // || executes the buffer. If the return is not 0, skip to the next
        // command, otherwise continue
        if (buffer->size > 0) {
          if (flgs->are_tokens) {
            execute_tok(buffer, flgs, bg_pids);
          } else {
            flgs->ret = execute(buffer, &flgs->pipe_fd);
          }
        }
        if (flgs->ret == 0) {
          clear_svec(buffer);
          ii = next_command(tokens, ii);
        }
      } else if (strcmp(token, "&&") == 0) {
        // && executes the buffer. If the return is 0, skip to the next command,
        // otherwise continue
        if (buffer->size > 0) {
          if (flgs->are_tokens) {
            execute_tok(buffer, flgs, bg_pids);
          } else {
            flgs->ret = execute(buffer, &flgs->pipe_fd);
          }
        }
        if (flgs->ret != 0) {
          clear_svec(buffer);
          ii = next_command(tokens, ii);
        }
      } else if (strcmp(token, "<") == 0 || strcmp(token, ">") == 0) {
        // file redirection
        ii++;
        if (flgs->are_tokens) {
          execute_red_tok(*token, buffer, tokens->data[ii], flgs, bg_pids);
        } else {
          flgs->ret =
              execute_red(*token, buffer, tokens->data[ii], &flgs->pipe_fd);
        }
      } else if (strcmp(token, "|") == 0) {
        // pipe
        if (flgs->are_tokens) {
          execute_pipe_tok(buffer, flgs, bg_pids);
        } else {
          flgs->pipe_fd = execute_pipe(buffer, &flgs->ret, &flgs->pipe_fd);
        }
      } else if (strcmp(token, "&") == 0) {
        // background execution
        if (ii < tokens->size - 1) {
          char* next = tokens->data[ii + 1];
          assert(strcmp(next, "&&") != 0 && strcmp(next, "||") != 0 &&
                 strcmp(next, "|") != 0);
          if (flgs->are_tokens) {
            execute_bg_tok(buffer, flgs, bg_pids);
          } else {
            vec_push_back(bg_pids, execute_bg(buffer, &flgs->pipe_fd));
          }
        }
      } else if (strcmp(token, "(") == 0) {
        // gets all the tokens in between the parentheses, stores them in the
        // buffer, set the tokens flag, and skips to next command.
        assert(buffer->size == 0);
        int next_cmd = next_command(tokens, ii);
        sub_svec(tokens, buffer, ii + 1, next_cmd - 1);
        flgs->are_tokens = 1;
        ii = next_cmd;
      } else if (strcmp(token, ")") == 0) {
        // if ( was handled correctly, you should never reach here unless there's a syntax issue.
        assert(0);
      } else if (strcmp(token, "\\") == 0) {
        // if there's a \ at the end, the \ processor didn't work correctly.
        assert(ii != tokens->size - 1);
      } else if (strcmp(token, "cd") == 0) {
        // change directory
        ii++;
        flgs->ret = chdir(tokens->data[ii]);
      } else if (strcmp(token, "exit") == 0) {
        // exit the program
        exit(flgs->ret);
      } else {
        svec_push_back(buffer, token);
      }
    }

    if (buffer->size > 0) {
      if (flgs->are_tokens) {
        execute_tok(buffer, flgs, bg_pids);
      } else {
        flgs->ret = execute(buffer, &flgs->pipe_fd);
      }
    }

    if (bg_mode) {
      free_svec(buffer);
      free(flgs);
      check_bg(bg_pids);
      _exit(0);
    }
  }

  return 0;
}

int main(int argc, char* argv[]) {
  FILE* script;
  // Opens script if provided
  if (argc > 1) {
    script = fopen(argv[1], "r");
  }

  svec* buffer = make_svec(1);
  cqueue* history = make_cqueue();
  vec* bg_pids = make_vec();
  char cmd[1024];
  cmd[0] = 0;
  flags* flgs = make_flags();

  while (1) {
    // Initial read
    if (argc == 1) {
      printf("nush$ ");
      fgets(cmd, 256, stdin);
    } else {
      fgets(cmd, 256, script);
    }

    fflush(stdout);

    svec* tokens = tokenize(cmd);
    // if \ is the last token, read more lines in until it's not
    while (tokens->size > 0 &&
           strcmp(tokens->data[tokens->size - 1], "\\") == 0) {
      if (argc == 1) {
        printf("      ");
        fgets(cmd, 256, stdin);
      } else {
        fgets(cmd, 256, script);
      }
      svec* next = tokenize(cmd);
      append_svec(tokens, next);
    }

    cmd[0] = 0;

    int cpid = execute_tokens(tokens, flgs, buffer, bg_pids, 0);
    // if tokens were executed as background process, store PID
    if (cpid != 0) {
      vec_push_back(bg_pids, cpid);
    }

    // Pushes tokens to the history, maybe a future feature implement?
    cqueue_push_back(history, tokens);

    // Check if EOF has been reached on the input and exits if so
    if (argc == 1 && feof(stdin) != 0) {
      free_svec(buffer);
      free_cqueue(history);
      int bg_ret = check_bg(bg_pids);
      int ret = flgs->ret;
      free(flgs);
      exit(ret ? ret : bg_ret);
    } else if (argc > 1 && feof(script) != 0) {
      free_svec(buffer);
      free_cqueue(history);
      fclose(script);
      int bg_ret = check_bg(bg_pids);
      int ret = flgs->ret;
      free(flgs);
      exit(ret ? ret : bg_ret);
    }
  }
}