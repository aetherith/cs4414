/* Thomas Foulds (tcf9bj)
 * 02/16/13
 * CS4414
 * thread.c
 */

#include "uthread.h"

void uthread_init(){
  // Initialize the thread queue
  list_init(&thread_queue, thread_compare_pri, thread_data_delete);

  // Initialize the handler thread
  getcontext(&handler);
  handler.uc_stack.ss_sp = &handler_stack;
  handler.uc_stack.ss_size = sizeof(handler_stack);
  handler.uc_link = NULL;
  makecontext(&handler, uthread_handler, 0);

  // Create a new thread to represent the calling process
  thread_t *new_thread = malloc( sizeof(thread_t) );

  // We initialize the calling thread as a zero priority
  thread_id = 0;
  thread_init(new_thread, 0, thread_id);
  thread_id++;
  
  // Insert the new thread into the thread queue
  list_insert_tail(&thread_queue, new_thread);

  // Now that we're done with setup we should gather the current context
  // into our thread's ucp.
  ucontext_t *new_context = &new_thread->ucp;
  getcontext(new_context);
};

int uthread_create(void (*func)(int), int val, int pri){
  // Create a new thread struct by switching to the handler context
  thread_t *new_thread = malloc( sizeof(thread_t) );
  thread_init(new_thread, pri, thread_id);
  thread_id++;

  // Set up context's stack
  new_thread->ucp.uc_stack.ss_sp = new_thread->ucp_stack;
  new_thread->ucp.uc_stack.ss_size = sizeof(new_thread->ucp_stack);
  // When the thread function returns we just exit
  new_thread->ucp.uc_link = NULL;

  // Make the new thread's context
  ucontext_t *new_context = &(new_thread->ucp);
  getcontext(new_context);
  makecontext(new_context, func, 1, val);
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
      ucontext_t *old_context = &cur_thread->ucp;
      ucontext_t *new_context = &new_thread->ucp;
      swapcontext(old_context, new_context);
    }
};

void uthread_exit(){  
  setcontext(&handler);
};

void uthread_handler(){
  while( 1 ){
    list_remove_head(&thread_queue);
    
    // If we have other threads to switch back to or want to return to the
    // creating context.
    if( thread_queue.length >= 1 )
      {
        list_item_t *new_head = thread_queue.head;
        thread_t *head_thread = (thread_t*)new_head->data;
        setcontext(&(head_thread->ucp));
      }
    // If there are no threads to switch to then we should just exit through the
    // handler's uc_link to NULL
    else break;
  }
};
