/* Thomas Foulds (tcf9bj)
 * 02/16/13
 * thread.c
 */

#include "uthread.h"

void uthread_init(){
  list_init(&thread_queue, thread_compare_pri, thread_data_delete);
};

int uthread_create(void (*func)(int), int val, int pri){

};

void uthread_yield(){

};

void uthread_exit(){

};
