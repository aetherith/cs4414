/* Thomas Foulds (tcf9bj)
 * 02/16/13
 * CS4414
 * thread.c
 */

#include "uthread.h"

void uthread_init(){
  // Initialize the thread queue
  list_init(&thread_queue, thread_compare_pri, thread_data_delete);

  // Gather the calling process into a context
  ucontext_t *cthread_context = malloc(sizeof(ucontext_t));
  getcontext(cthread_context);

  // Create a new thread to represent the calling process
  thread_t *new_thread = malloc(sizeof(thread_t));
 
  // We initialize the calling thread as a zero priority
  thread_id = 0;
  thread_init(new_thread, cthread_context, 0, thread_id);
  thread_id++;

  // Because our stack is not dynamically allocated we don't want to try
  // and free() it when we remove the node.
  new_thread->free_stack = false;
  
  // Insert the new thread into the thread queue
  list_insert_tail(&thread_queue, new_thread);
};

int uthread_create(void (*func)(int), int val, int pri){
  ucontext_t *ucp = malloc(sizeof(ucontext_t));
  // Allocate a 1MB stack
  char *ucp_stack = malloc(1048576 * sizeof(char));
  getcontext(ucp);
    
  ucp->uc_stack.ss_sp = ucp_stack;
  ucp->uc_stack.ss_size = 1048576;

  // Hopefully this makes it so when a thread returns it just exits
  ucp->uc_link = NULL;

  makecontext(ucp, func, 1, val);

  // Create a new thread struct
  thread_t *new_thread = malloc(sizeof(thread_t));
  thread_init(new_thread, ucp, pri, thread_id);
  thread_id++;

  // Insert the new thread into the thread queue
  list_insert_tail(&thread_queue, new_thread);

  return thread_id - 1;
};

void uthread_yield(){
  if( thread_queue.length > 1)
    {
      list_item_t *cur_head = thread_queue.head;
      thread_t *cur_thread = (thread_t*)cur_head->data;

      // Rotate the thread queue to bring the next thread to run to the head.
      list_rotate_one(&thread_queue);

      // Swap the new head context into running.
      list_item_t *new_head = thread_queue.head;
      thread_t *new_thread = (thread_t*)new_head->data;
      swapcontext(cur_thread->ucp, new_thread->ucp);
    }
};

void uthread_exit(){  
  // Remove head from the thread queue to get rid of currently running
  // thread's information.
  printf("Thread ID %i is exiting.\n", ((thread_t*)thread_queue.head->data)->id);
  list_remove_head(&thread_queue);
  // If we still have threads to switch to
  if( thread_queue.length >= 1 )
    {
      // Swap the new head context into running.
      list_item_t *new_head = thread_queue.head;
      thread_t *new_thread = (thread_t*)new_head->data;
      setcontext(new_thread->ucp);
    }
};

void uthread_dinit(){
  while( thread_queue.length > 0 )
      list_remove_head(&thread_queue);
};
