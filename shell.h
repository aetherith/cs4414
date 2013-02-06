/* Thomas Foulds
 * tcf9bj
 * 02/01/13
 * CS4414
 * shell.h
 */

#ifndef SHELL_H
#define SHELL_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "process.h"

// Return the position of the first redirection type token
// The token_buf_len is the *actual* number of valid tokens in the buffer, not the zero-index position of the last token
// For all error states, return 0
//  * Redirector is first token encountered
//  * Start position is beyond the buffer bounds
//  * No redirector is found
unsigned int find_redirect(char **token_buf, unsigned int *token_buf_len, unsigned int *start_pos);

// Return the positon of the first pipe token after the given start position
// The token_buf_len is the *actual* number of valid tokens in the buffer, not the zero-index position of the last token
// For all error states, return 0 in a similar fashion to find_redirect()
unsigned int find_pipe(char **token_buf, unsigned int *token_buf_len, unsigned int *start_pos);

#endif
