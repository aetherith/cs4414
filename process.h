/* Thomas Foulds
 * tcf9bj
 * 02/06/13
 * CS4414
 * process.h
 */

#ifndef PROCESS_H
#define PROCESS_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>

typedef struct process{
  char **params;
  bool is_pipe_read;
  bool is_pipe_write;
  // Takes on 4 values
  //  * 0 - no redirection or output redirection superceeded by pipe
  //  * 1 - overwrite output
  //  * 2 - append output
  //  * 3 - read input
  int redirect_type;
  char *in_file;
  char *out_file;
} process_t;

void process_init(process_t *process);
#endif
