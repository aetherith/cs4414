/* Thomas Foulds (tcf9bj)
 * 02/16/13
 * thread.c
 */

#include "uthread.h"

void uthread_init(){
  // Initialize the thread queue
  list_init(&thread_queue, thread_compare_pri, thread_data_delete);
  // Set the current thread of execution into the queue
  ucontext_t *calling_thread = malloc(sizeof(ucontext_t));
  getcontext(calling_thread);

};

int uthread_create(void (*func)(int), int val, int pri){
  ucontext_t *ucp = malloc(sizeof(ucontext_t));
  char *ucp_stack = malloc(1048576 * sizeof(char));
  getcontext(ucp);
  ucp->uc_stack.ss_sp = ucp_stack;
  ucp->uc_stack.ss_size = sizeof(*ucp_stack);
};

void uthread_yield(){

};

void uthread_exit(){

};
