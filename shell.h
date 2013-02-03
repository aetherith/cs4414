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

// Return the position of the first redirection type token
// The token_buf_len is the *actual* number of valid tokens in the buffer, not the zero-index position of the last token
// If the first token is a redirector or one is not found, return 0
unsigned int find_redirect(char **token_buf, unsigned int *token_buf_len);

#endif
