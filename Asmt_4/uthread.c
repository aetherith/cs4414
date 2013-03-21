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

#ifdef __APPLE__
#define _XOPEN_SOURCE		// suppress warning for ucontext.h (deprecated)
#endif

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <ucontext.h>
#include <signal.h>

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

static uthread_t *current_thread = NULL;
static uthread_t *tail_thread = NULL;
static uthread_t *head_thread = NULL;
static int uthread_id = 0;
static int live_uthreads = 0;

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
  main_thread->pri = 0;

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
}

int
uthread_create (uthread_func_t func, int val, int pri)
{
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
      thread_buf_pos++;
    }
  thread_buf[thread_buf_pos] = current_thread;
  thread_buf[thread_buf_pos + 1] = new_thread;

  qsort(thread_buf, live_uthreads, sizeof(uthread_t*), uthread_priority_sort);
  /* iterate through the array and retie next pointers */
  /* highest priority thread ends up in position 0 of the thread buffer */
  head_thread = thread_buf[0];
  for( thread_buf_pos = 1; thread_buf_pos < live_uthreads; thread_buf_pos++ )
      thread_buf[thread_buf_pos - 1]->next = thread_buf[thread_buf_pos];
  /* close the circle of threads back on itself */
  thread_buf[thread_buf_pos]->next = head_thread;
  tail_thread = thread_buf[thread_buf_pos];

  /* test that the circle is closed */
  assert (tail_thread->next == head_thread);

  return ++uthread_id;
}

void
uthread_yield (void)
{
  if (current_thread == current_thread->next)
    {
      return;
    }

  tail_thread = current_thread;
  current_thread = current_thread->next;

  swapcontext (&tail_thread->context, &current_thread->context);
}

void
uthread_exit (void)
{
  /* How do I deal with removing a thread that may not be at the head? */
  assert (tail_thread->next == head_thread);
  uthread_t *old_thread = current_thread;
  current_thread = current_thread->next;
  tail_thread->next = current_thread;

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