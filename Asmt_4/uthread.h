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
 * uthread.h
 *
 */

/**
 * Thomas Foulds (tcf9bj)
 * 03-27-2013
 * CS4414
 * Assignment 4
 */

#ifndef __UTHREAD_H
#define __UTHREAD_H

#include <ucontext.h>

typedef void (*uthread_func_t) (int val);

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
  struct uthread_wait_record *next;

} uthread_wait_record_t;

typedef struct uthread_mutex
{
  uthread_t *locking_thread;
  int wait_queue_length;
  /* head of a single link list of waiting threads */
  uthread_wait_record_t *wait_queue_head;
  uthread_wait_record_t *wait_queue_tail;
} uthread_mutex_t;

void uthread_init (void);
int uthread_create (uthread_func_t func, int val, int pri);
void uthread_yield (void);
void uthread_exit (void);
int uthread_priority_sort(const void *key, const void *with);
void uthread_yield_handler(int signum);

/* mutex methods */
void uthread_mutex_init(uthread_mutex_t *lockVar);
void uthread_mutex_lock(uthread_mutex_t *lockVar);
void uthread_mutex_unlock(uthread_mutex_t *lockVar);
void uthread_mutex_destroy(uthread_mutex_t *lockVar);
void uthread_wait_record_init(uthread_wait_record_t *record);
int uthread_wait_record_priority_sort(const void *key, const void *with);

#endif //__UTHREAD_H
