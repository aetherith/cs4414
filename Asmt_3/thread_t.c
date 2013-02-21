/* Thomas Foulds (tcf9bj)
 * 02/18/13
 * CS4414
 * thread_t.c
 */

#include "thread_t.h"

void thread_init(thread_t *t,
                 ucontext_t *ucp,
                 int pri, int id){
  t->ucp = ucp;
  t->pri = pri;
  t->id = id;
  t->free_stack = true;
};

// Clean up the thread object when it is deleted from the linked list
void thread_data_delete(void *item){
  thread_t *t = (thread_t*)(((list_item_t*)item)->data);
  // If the thread's stack was dynamically allocated, i.e. the thread
  // was created using uthread_create() we must deallocate its stack space.
  if( t->free_stack == true ) 
    {
      // printf("Freeing stack!\n");
      // For some reason it doesn't seem to think that the stack was dynamically
      // allocated.  Problem is that this memory leaks out until reclaimed by OS.
      // free(t->ucp->uc_stack.ss_sp);
      // printf("Stack freed!\n");
    }
  // Deallocate the dynamically created ucontext_t object
  free(t->ucp);
  // Deallocate the thread object itself
  free(t);
};

// Compare two thread objects based on their priority value
int thread_compare_pri(const void *key, const void *with){
  list_item_t **arg1 = (list_item_t**)key;
  list_item_t **arg2 = (list_item_t**)with;
  thread_t *t1 = (thread_t*)((*arg1)->data);
  thread_t *t2 = (thread_t*)((*arg2)->data);
  int pri1 = t1->pri;
  int pri2 = t2->pri;
  return pri1 - pri2;
};
