/**
 * Copyright 2012 University of Virginia. All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without modification, are
 * permitted provided that the following conditions are met:
 * 
 * 1. Redistributions of source code must retain the above copyright notice, this list of
 * conditions and the following disclaimer.
 * 
 * 2. Redistributions in binary form must reproduce the above copyright notice, this list
 * of conditions and the following disclaimer in the documentation and/or other materials
 * provided with the distribution.
 * 
 * THIS SOFTWARE IS PROVIDED BY <COPYRIGHT HOLDER> ''AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
 * FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/**
 * CS 4414 Spring 2012
 * Lab 3: Creating a User-Level Thread Package Part I
 *
 * Reference Implementation
 *
 * @author Zhengyang Liu (zl4ef)
 *
 * uthread.c
 *
 */

/**
 * Thomas Foulds (tcf9bj)
 * 03-27-2013
 * CS4414
 * Assignment 4
 */

#ifdef __APPLE__
#define _XOPEN_SOURCE		// suppress warning for ucontext.h (deprecated)
#endif

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <ucontext.h>
#include <signal.h>
#include <sys/time.h>

#include "uthread.h"

#define UTHREAD_STACK_SIZE	(1 << 20)

typedef struct uthread
{
  ucontext_t context;

  /* context.uc_stack.ss_sp maybe stack base or stack pointer (platform dependent),
     (e.g. on OS X, uc_stack.ss_sp is the stack pointer and thus changes as the user context progresses,
     on most other *IX systems, uc_stack.ss_sp is the stack base and does not change)
     thus we save the stack base here */
  void *stack;

  /* priority of the thread 0-9 */
  int pri;

  /* maintain a circular list */
  struct uthread *next;
} uthread_t;

typedef struct uthread_wait_record
{
  uthread_t *thread;
  uthread_wait_record_t *next;

} uthread_wait_record_t;

typedef uthread_mutex
{
  uthread_t *locking_uthread;
  int wait_queue_length;
  /* head of a single link list of waiting threads */
  uthread_wait_record_t *wait_queue_head;
  uthread_wait_record_t *wait_queue_tail;
} uthread_mutex_t;

static uthread_t *current_thread = NULL;
static uthread_t *tail_thread = NULL;
static uthread_t *head_thread = NULL;
static int uthread_id = 0;
static int live_uthreads = 0;
static struct itimerval yield_timer;
static int TIME_SLICE_USEC = 10;
static struct sigaction yield_action;
static struct sigaction old_sig_action;

/**
 * on OS X and FreeBSD, you can free current context's stack
 * on most Linux distros, it seems you have to do that on a different context
 */
#if !(defined __APPLE__ || defined __FreeBSD__)
// this solution came from previous TAs Jeff Kusi and Keaton Monger
// set up a separate context just for freeing stacks
ucontext_t cleanup_context;
static uthread_t *retired_thread;	// the thread to be free'd
static char cleanup_stack[UTHREAD_STACK_SIZE];	// this is allocated on the heap, thus we don't need to free it

void
free_stack (void)
{
  free (retired_thread->stack);
  free (retired_thread);

  if (retired_thread == current_thread)
    {
      exit (0);
    }
  setcontext (&current_thread->context);
}
#endif

void
uthread_init (void)
{
  printf("== uthread_init() ==\n");
  if (current_thread)
    {
      return;
    }

  uthread_t *main_thread = (uthread_t *) malloc (sizeof (uthread_t));

  if (getcontext (&main_thread->context) < 0)
    {
      perror ("getcontext");
      exit (-1);
    }

  main_thread->stack = NULL;	// prevent accidental free
  main_thread->next = main_thread;
  tail_thread = current_thread = main_thread;

  /* set calling thread's priority to the lowest available */
  main_thread->pri = 9;

  /* increment the number of live uthreads */
  live_uthreads++;

#if !(defined __APPLE__ || defined __FreeBSD__)
  if (getcontext (&cleanup_context) < 0)
    {
      perror ("getcontext");
      exit (-1);
    }

  cleanup_context.uc_stack.ss_sp = cleanup_stack;
  cleanup_context.uc_stack.ss_size = UTHREAD_STACK_SIZE;
  cleanup_context.uc_link = NULL;

  makecontext (&cleanup_context, free_stack, 0);
#endif

  /* since timers are process wide, here is where we set one up */
  yield_timer.it_interval.tv_sec = 0;
  yield_timer.it_interval.tv_usec = TIME_SLICE_USEC;
  yield_timer.it_value.tv_sec = 0;
  yield_timer.it_value.tv_usec = TIME_SLICE_USEC;
  if( setitimer(ITIMER_VIRTUAL, &yield_timer, NULL) < 0 )
    {
      perror("timer");
      exit(-1);
    }

  /* now to configure a signal handler for SIGVTALRM */
  yield_action.sa_handler = &uthread_yield_handler;
  sigemptyset( &yield_action.sa_mask );
  yield_action.sa_flags = 0;
  if( sigaction(SIGVTALRM, &yield_action, NULL) < 0 )
    {
      perror("sigaction");
      exit(-1);
    }

  printf("!= uthread_init() =!\n");
}

int
uthread_create (uthread_func_t func, int val, int pri)
{
  printf("== uthread_create() ==\n");
  uthread_t *new_thread = (uthread_t *) malloc (sizeof (uthread_t));

  if (getcontext (&new_thread->context) < 0)
    {
      perror ("getcontext");
      exit (-1);
    }

  new_thread->context.uc_stack.ss_size = UTHREAD_STACK_SIZE;
  new_thread->context.uc_stack.ss_sp = new_thread->stack =
    malloc (UTHREAD_STACK_SIZE);
  new_thread->context.uc_link = NULL;

  new_thread->pri = pri;

  makecontext (&new_thread->context, (void (*)(void)) func, 1, val);

  /* increment number of live uthreads */
  live_uthreads++;

  /* copy thread pointers into an array and quicksort them by priority */
  uthread_t **thread_buf = malloc( live_uthreads * sizeof(uthread_t*) );
  uthread_t *thread_copy_p = current_thread->next;
  int thread_buf_pos = 0;
  
  //printf("= Copy thread pointers into array =\n");

  while( thread_copy_p != current_thread )
    {
      thread_buf[thread_buf_pos] = thread_copy_p;
      thread_copy_p = thread_copy_p->next;
      thread_buf_pos++;
    }
  thread_buf[thread_buf_pos] = current_thread;
  thread_buf[thread_buf_pos + 1] = new_thread;

  //printf("! Copy thread pointers into array !\n= Quicksort =\n");

  qsort(thread_buf, live_uthreads, sizeof(uthread_t*), uthread_priority_sort);

  //printf("! Quicksort !\n");

  /* iterate through the array and retie next pointers */
  /* highest priority thread ends up in position 0 of the thread buffer */
  
  //printf("= Retie pointers =\n");

  head_thread = thread_buf[0];
  for( thread_buf_pos = 1; thread_buf_pos < live_uthreads; thread_buf_pos++ )
    {
      thread_buf[thread_buf_pos - 1]->next = thread_buf[thread_buf_pos];
    }
  /* close the circle of threads back on itself */
  thread_buf[thread_buf_pos - 1]->next = head_thread;
  tail_thread = thread_buf[thread_buf_pos - 1];

  //printf("! Retie pointers !\n");

  /* test that the circle is closed */
  assert (tail_thread->next == head_thread);

  /* print out the run queue in order
  uthread_t* thread_print_p = head_thread;
  printf("HEAD\n");
  while( thread_print_p != tail_thread )
    {
      printf("Pri: %i\n", thread_print_p->pri);
      thread_print_p = thread_print_p->next;
    }
  printf("Pri: %i\nTAIL\n", thread_print_p->pri);
  */

  /* free the thread buffer as we don't need it anymore */
  free(thread_buf);

  printf("!= uthread_create() =!\n");
  return ++uthread_id;
}

void
uthread_yield (void)
{
  printf("== uthread_yield() ==\n");

  if (current_thread == current_thread->next)
    {
      printf("!= uthread_yield() - No Contest =!\n");
      return;
    }

  uthread_t *old_thread = current_thread;
  if( old_thread->pri < old_thread->next->pri )
    {
      if( current_thread == head_thread )
        {
          printf("!= uthread_yield() - Highest available thread =!\n");
          return;
        }

      else
        {
          printf("= Looping to head within priority class %i =\n",
                 current_thread->pri);
          current_thread = head_thread;
        }
    }
  else
    {
      printf("= Yielding from priority %i to priority %i =\n",
             old_thread->pri, old_thread->next->pri);
      current_thread = old_thread->next;
    }
  printf("!= uthread_yield() =!\n");

  swapcontext (&old_thread->context, &current_thread->context);
}

void
uthread_exit (void)
{
  printf("== uthread_exit() ==\n");
  uthread_t *old_thread = current_thread;

  /* change to next highest priority thread */
  if( old_thread == head_thread )
    {
      /* if we're executing the current highest priority thread */
      printf("= Exiting the head thread =\n");

      current_thread = current_thread->next;
      tail_thread->next = current_thread;
      head_thread = current_thread;
    }
  else
    {
      /* otherwise, a higher priority thread has yielded and we're midway */
      uthread_t *prev_thread = head_thread;
      
      /* find the thread directly before the currently running one */
      while( prev_thread->next != old_thread )
        prev_thread = prev_thread->next;
      
      if( old_thread->pri < old_thread->next->pri )
        {
          /* the next thread in the cycle is of a "lower" priority so we want
           * to loop around and start executing the current head_thread
           */
          printf("= Exiting at end of priority class =\n");

          prev_thread->next = old_thread->next;
          current_thread = head_thread;
        }
      else
        {
          /* the next thread is of equal or higher priority to the current one.
           * switch and begin executing it.
           */
          if( old_thread == tail_thread )
            {
              /* if we're at the tail patch things up with those pointers */
              printf("= Exiting the tail thread =\n");
              tail_thread = prev_thread;
              prev_thread->next = head_thread;
              current_thread = head_thread;
            }
          else
            {
              current_thread = current_thread->next;
              prev_thread->next = current_thread;
            }
        }
    }

  printf("!= uthread_exit() =!\n");
#if defined __APPLE__ || defined __FreeBSD__
  free (old_thread->stack);
  free (old_thread);
  if (current_thread == old_thread)
    {
      exit (0);
    }
  setcontext (&current_thread->context);
#else
  retired_thread = old_thread;
  setcontext (&cleanup_context);
#endif
}

int uthread_priority_sort(const void *key, const void *with)
{
  uthread_t *arg1 = *((uthread_t**)key);
  uthread_t *arg2 = *((uthread_t**)with);
  //printf("Arg1: %i VS Arg2: %i\n", arg1->pri, arg2->pri);
  if( arg1->pri < arg2->pri ) return -1;
  if( arg1->pri > arg2->pri ) return 1;
  return 0;
}

void uthread_yield_handler( int signum )
{
  printf("== Timeout yield ==\n");
  uthread_yield();
}

void uthread_mutex_init(uthread_mutex_t *lockVar)
{
  lockVar->locking_uthread = NULL;
  lockVar->wait_queue_length = 0;
  lockVar->wait_queue_head = NULL;
  lockVar->wait_queue_tail = NULL;
}

void uthread_mutex_lock(uthread_mutex_t *lockVar)
{
  sigset_t mask, orig_mask;
  sigfillset(&mask);
  /* block all signals */
  if( sigprocmask(SIG_BLOCK, &mask, &orig_mask) < 0 )
    {
      perror("block signals");
      exit(-1);
    }
  uthread_wait_record_t *new_wait = malloc(sizeof(uthread_wait_record_t));
  uthread_wait_record_init(new_wait);
  lockVar->wait_queue_length++;
  /* if there is only one thread trying to lock this mutex */
  if( lockVar->wait_queue_length == 1 )
    {
      lockVar->wait_queue_head = new_wait;
      lockVar->wait_queue_tail = new_wait;
      lockVar->locking_thread = new_wait->thread;
    }
  else
    {

    }
  /* reset the signal mask to the original */
  if( sigprocmask(SIG_SETMASK, &orig_mask, NULL) < 0 )
    {
      perror("unblock signals");
      exit(-1);
    }
  while(current_thread != lockVar->locking_uthread) uthread_yield();
}

void uthread_mutex_unlock(uthread_mutex_t *lockVar)
{

}

void uthread_mutex_destroy(uthread_mutex_t *lockVar)
{

}

void uthread_wait_record_init(uthread_wait_record_t *record)
{
  record->thread = current_thread;
  record->next = NULL:
}

int uthread_wait_record_priority_sort(const void *key, const void *with)
{
  return -1;
}
