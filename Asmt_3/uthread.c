/* Thomas Foulds (tcf9bj)
 * 02/16/13
 * thread.c
 */

#include "thread.h"

void uthread_init(){
  list_init(&thread_queue);
};

int uthread_create(void (*func)(int), int val, int pri){

};

void uthread_yield(){

};

void uthread_exit(){

};
