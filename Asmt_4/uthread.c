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
 * Amanda Ray (ajr2fu)
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
  //printf("== uthread_init() ==\n");
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

  //printf("!= uthread_init() =!\n");
}

int
uthread_create (uthread_func_t func, int val, int pri)
{
  //printf("== uthread_create() ==\n");
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

  while( thread_copy_p != current_thread )
    {
      thread_buf[thread_buf_pos] = thread_copy_p;
      thread_copy_p = thread_copy_p->next;
      thread_buf_pos++;
    }
  thread_buf[thread_buf_pos] = current_thread;
  thread_buf[thread_buf_pos + 1] = new_thread;

  qsort(thread_buf, live_uthreads, sizeof(uthread_t*), uthread_priority_sort);

  /* iterate through the array and retie next pointers */
  /* highest priority thread ends up in position 0 of the thread buffer */
  
  head_thread = thread_buf[0];
  for( thread_buf_pos = 1; thread_buf_pos < live_uthreads; thread_buf_pos++ )
    {
      thread_buf[thread_buf_pos - 1]->next = thread_buf[thread_buf_pos];
    }
  /* close the circle of threads back on itself */
  thread_buf[thread_buf_pos - 1]->next = head_thread;
  tail_thread = thread_buf[thread_buf_pos - 1];

  /* test that the circle is closed */
  assert (tail_thread->next == head_thread);

  /* free the thread buffer as we don't need it anymore */
  free(thread_buf);

  //printf("!= uthread_create() =!\n");
  return ++uthread_id;
}

void
uthread_yield (void)
{
  //printf("== uthread_yield() ==\n");

  if (current_thread == current_thread->next)
      return;

  uthread_t *old_thread = current_thread;
  if( old_thread->pri < old_thread->next->pri )
    {
      if( current_thread == head_thread )
          return;
      else
          current_thread = head_thread;
    }
  else
      current_thread = old_thread->next;
  //printf("!= uthread_yield() =!\n");

  swapcontext (&old_thread->context, &current_thread->context);
}

void
uthread_exit (void)
{
  //printf("== uthread_exit() ==\n");
  uthread_t *old_thread = current_thread;

  /* change to next highest priority thread */
  if( old_thread == head_thread )
    {
      /* if we're executing the current highest priority thread */
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

  //printf("!= uthread_exit() =!\n");
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
  if( arg1->pri < arg2->pri ) return -1;
  if( arg1->pri > arg2->pri ) return 1;
  return 0;
}

void uthread_yield_handler( int signum )
{
  uthread_yield();
}

void uthread_mutex_init(uthread_mutex_t *lockVar)
{
  //printf("== uthread_mutex_init() ==\n");
  lockVar->locking_thread = NULL;
  lockVar->wait_queue_length = 0;
  lockVar->wait_queue_head = NULL;
  lockVar->wait_queue_tail = NULL;
  //printf("!= uthread_mutex_init() =!\n");
}

void uthread_mutex_lock(uthread_mutex_t *lockVar)
{
  //printf("== uthread_mutex_lock ==\n");
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
      /* insert new wait record and sort */
      uthread_wait_record_t **wait_buf = malloc(lockVar->wait_queue_length *
                                                sizeof(uthread_wait_record_t*));
      uthread_wait_record_t *copy_p = lockVar->wait_queue_head;
      int wait_buf_pos = 0;
      while( copy_p != NULL )
        {
          wait_buf[wait_buf_pos] = copy_p;
          copy_p = copy_p->next;
          wait_buf_pos++;
        }
      wait_buf[wait_buf_pos] = new_wait;
      qsort(wait_buf, lockVar->wait_queue_length, sizeof(uthread_wait_record_t*),
            uthread_wait_record_priority_sort);
      /* reset the pointers for head and tail */
      lockVar->wait_queue_head = wait_buf[0];
      lockVar->wait_queue_tail = wait_buf[lockVar->wait_queue_length - 1];
      lockVar->wait_queue_tail->next = NULL;
      for( wait_buf_pos = 1; wait_buf_pos < lockVar->wait_queue_length;
           wait_buf_pos++)
        {
          wait_buf[wait_buf_pos - 1]->next = wait_buf[wait_buf_pos];
        }
      free(wait_buf);
    }
  /* reset the signal mask to the original */
  if( sigprocmask(SIG_SETMASK, &orig_mask, NULL) < 0 )
    {
      perror("unblock signals");
      exit(-1);
    }
  //printf("!= uthread_mutex_lock() =!\n");
  while(current_thread != lockVar->locking_thread) uthread_yield();
}

void uthread_mutex_unlock(uthread_mutex_t *lockVar)
{
  //printf("== uthread_mutex_unlock() ==\n");
  sigset_t mask, orig_mask;
  sigfillset(&mask);
  /* block all signals */
  if( sigprocmask(SIG_BLOCK, &mask, &orig_mask) < 0 )
    {
      perror("block signals");
      exit(-1);
    }
  uthread_wait_record_t *old_record;
  if( lockVar->wait_queue_length == 1 )
    {
      /* if the unlocking thread is the only one waiting */
      old_record = lockVar->wait_queue_head;
      lockVar->wait_queue_head = NULL;
      lockVar->wait_queue_tail = NULL;
    }
  else
    {
      if( lockVar->wait_queue_head->thread == lockVar->locking_thread )
        {
          /* if the unlocking thread is the head of the wait queue */
          old_record = lockVar->wait_queue_head;
          lockVar->wait_queue_head = old_record->next;
        }
      else
        {
          /* if our current thread is not the highest priority one in the
           * queue.  Namely it has been sorted further down in the list.
           */
          uthread_wait_record_t *search_prev_p = lockVar->wait_queue_head;
          uthread_wait_record_t *search_p = lockVar->wait_queue_head->next;
          while( search_p->thread != lockVar->locking_thread )
            {
              search_prev_p = search_p;
              search_p = search_p->next;
              if( search_p == NULL)
                {
                  perror("wait record not found");
                  exit(-1);
                }
            }
          search_prev_p->next = search_p->next;
          old_record = search_p;
        }
      lockVar->locking_thread = lockVar->wait_queue_head->thread;
    }
  free(old_record);
  lockVar->wait_queue_length--;
  if( sigprocmask(SIG_SETMASK, &orig_mask, NULL) < 0 )
    {
      perror("unblocking signals");
      exit(-1);
    }
  //printf("!= uthread_mutex_unlock =!\n");
}

void uthread_mutex_destroy(uthread_mutex_t *lockVar)
{
  uthread_wait_record_t *free_p = lockVar->wait_queue_head;
  while( lockVar->wait_queue_head != NULL )
    {
      lockVar->wait_queue_head = lockVar->wait_queue_head->next;
      free(free_p);
      free_p = lockVar->wait_queue_head;
    }
}

void uthread_wait_record_init(uthread_wait_record_t *record)
{
  //printf("== uthread_wait_record_init() ==\n");
  record->thread = current_thread;
  record->next = NULL;
  //printf("!= uthread_wait_record_init() =!\n");
}

int uthread_wait_record_priority_sort(const void *key, const void *with)
{
  uthread_wait_record_t *arg1 = *((uthread_wait_record_t**)key);
  uthread_wait_record_t *arg2 = *((uthread_wait_record_t**)with);
  if( arg1->thread->pri < arg2->thread->pri ) return -1;
  if( arg1->thread->pri > arg2->thread->pri ) return 1;
  return 0;
}
