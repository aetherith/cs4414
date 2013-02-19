/* Thomas Foulds (tcf9bj)
 * 02/16/13
 * thread.h
 */

#ifndef THREAD_H
#define THREAD_H

#include <sys/ucontext.h>
#include "list_t.h"
#include "thread_t.h"

list_t thread_queue;

// Initialize the thread queue
void uthread_init();

// Create a new thread with:
// pri - Thread priority
// void (*func)(int) - Function pointer to thread main
// val - value to pass to thread
int uthread_create(void (*func)(int), int val, int pri);

// Instructs the currently running thread to give up access to the CPU
// The thread is then pushed onto the end of the scheduling queue and the head
// of the queue is popped off and allowed to begin executing.
void uthread_yield();

// Terminate the currently running thread and allow the head of the scheduling
// queue to begin running.
void uthread_exit();

#endif
