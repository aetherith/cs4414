/* Thomas Foulds (tcf9bj)
 * 02/18/13
 * CS4414
 * thread_t.c
 */

#include "thread_t.h"

void thread_init(thread_t *t,
                 int pri, int id){
  t->pri = pri;
  t->id = id;
};

// Clean up the thread object when it is deleted from the linked list
void thread_data_delete(void *item){
  thread_t *t = (thread_t*)(((list_item_t*)item)->data);
  // Deallocate the thread object itself as the context and the stack
  // are statically allocated in the thread struct.
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
