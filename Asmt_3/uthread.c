/* Thomas Foulds (tcf9bj)
 * 02/16/13
 * thread.c
 */

#include "uthread.h"

void uthread_init(){
  printf("== UThread Init ==\n");

  // Initialize the thread queue
  list_init(&thread_queue, thread_compare_pri, thread_data_delete);
  // Gather the calling process into a context
  ucontext_t *cthread_context = malloc(sizeof(ucontext_t));
  getcontext(cthread_context);
  // Create a new thread to represent the calling process
  thread_t *new_thread = malloc(sizeof(thread_t));
  // We initialize the calling thread as a zero priority
  thread_init(new_thread, cthread_context, 0);
  // Because our stack is not dynamically allocated we don't want to try
  // and free() it when we remove the node.
  new_thread->free_stack = false;
  // Insert the new thread into the thread queue
  list_insert_tail(&thread_queue, new_thread);

  printf("!= UThread Init =!\n");
};

int uthread_create(void (*func)(int), int val, int pri){
  printf("== UThread Create ==\n");

  ucontext_t *ucp = malloc(sizeof(ucontext_t));
  // Allocate a 1MB stack
  char *ucp_stack = malloc(1048576 * sizeof(char));
  getcontext(ucp);
  
  printf("Got context.\n");
  
  ucp->uc_stack.ss_sp = ucp_stack;
  ucp->uc_stack.ss_size = 1048576;

  printf("Set up stack.\n");

  // Hopefully this makes it so when a thread returns it just exits
  ucp->uc_link = NULL;

  printf("Set uc_link to NULL.\n");

  makecontext(ucp, func, 1, val);

  printf("Made context.\n");

  // Create a new thread struct
  thread_t *new_thread = malloc(sizeof(thread_t));
  thread_init(new_thread, ucp, pri);
  // Insert the new thread into the thread queue
  list_insert_tail(&thread_queue, new_thread);

  printf("!= UThread Create =!\n");
};

void uthread_yield(){
  printf("== UThread Yield ==\n");

  list_item_t *cur_head = thread_queue.head;
  thread_t *cur_thread = (thread_t*)cur_head->data;
  // Save current thread's status back to its thread struct
  getcontext(cur_thread->ucp);

  printf("Saved current context.\n");

  // Rotate the thread queue to bring the next thread to run to the head.
  list_rotate_one(&thread_queue);

  printf("Rotated queue.\n");

  // Swap the new head context into running.
  list_item_t *new_head = thread_queue.head;
  thread_t *new_thread = (thread_t*)new_head->data;
  setcontext(new_thread->ucp);

  printf("Set new context.\n");
  printf("!= UThread Yield =!\n");
};

void uthread_exit(){
  printf("== UThread Exit ==\n");
  
  // Remove head from the thread queue to get rid of currently running
  // thread's information.

  // Swap the new head context into running.
  
  printf("!= UThread Exit =!\n");
};
