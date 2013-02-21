/* Thomas Foulds (tcf9bj)
 * 02/18/13
 * CS4414
 * thread_t.h
 */

#ifndef THREAD_T_H
#define THREAD_T_H

#include <stdlib.h>
#include <stdbool.h>
#include <sys/ucontext.h>
#include "list_item_t.h"

typedef struct thread{
  ucontext_t *ucp;
  int pri;
  int id;
  bool free_stack;
} thread_t;

void thread_init(thread_t *t,
                 ucontext_t *ucp,
                 int pri,
                 int id);

// Data delete method for use with linked list
void thread_data_delete(void *item);

// Item comparision method to use with linked list
// Orders based on priority value.
int thread_compare_pri(const void *key, const void *with);

#endif
