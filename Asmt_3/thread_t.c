/* Thomas Foulds (tcf9bj)
 * 02/18/13
 * CS4414
 * thread_t.c
 */

#include "thread_t.h"

void thread_init(thread_t *t,
                 ucontext_t *ucp,
                 void (*main)(int val),
                 int val,
                 int pri){
  t->ucp = ucp;
  t->main = main;
  t->val = val;
  t->pri = pri;
};

void thread_data_delete(void *item){
  // I'm not certain if there is any dynamically allocated data inside the thread object so we're leaving the minimal stub for now
  free(((list_item_t*)item)->data);
};

int thread_compare_pri(const void *key, const void *with){
  list_item_t **arg1 = (list_item_t**)key;
  list_item_t **arg2 = (list_item_t**)with;
  thread_t *t1 = (thread_t*)((*arg1)->data);
  thread_t *t2 = (thread_t*)((*arg2)->data);
  int pri1 = t1->pri;
  int pri2 = t2->pri;
  return pri1 - pri2;
};
