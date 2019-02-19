#ifndef FLAGS_H
#define FLAGS_H

/**
 * @brief Stores the various flags used by {@code nush}.
 */
typedef struct flags {
    int ret;
    int are_tokens; // if the buffer is a command or a bunch of tokens, including operators
    int pipe_fd; // The file descriptor of a pipe to be used as input. 0 if no such pipe exists.
} flags;

/**
 * @brief Creates a {@code flags} with the default flags.
 */
flags* make_flags();

#endif