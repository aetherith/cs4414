/* Thomas Foulds
 * tcf9bj
 * 02/06/13
 * process.c
 */

#include "process.h"

void process_init(process_t *process)
{
  process->params = NULL;
  process->is_pipe_read = false;
  process->is_pipe_write = false;
  process->redirect_type = 0;
  process->in_file = NULL;
  process->out_file = NULL;
};
