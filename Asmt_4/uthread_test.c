/* Code to test the uthread library */

#include <stdio.h>
#include <unistd.h>
#include "uthread.h"

void threadFunc (int);

int
main ()
{
  int i = 0;

  uthread_init ();

  uthread_create (threadFunc, 1, 0);
  uthread_create (threadFunc, 2, 0);
  uthread_create (threadFunc, 3, 0);
  uthread_create (threadFunc, 4, 0);

  // uncomment to test uthread_exit() on main earlier than others
  // uthread_exit();

  for (i = 0; i < 10; i++)
    {
      printf ("This is the main function\n");
      uthread_yield ();
    }
  printf ("I am main and I am exiting\n");

  uthread_exit ();

  printf ("You shall not reach this line\n");
  return 0;
}

/* Lame user thread */
void
threadFunc (int val)
{
  int i, j;

  for (i = 0; i < 5; i++)
    {
      printf ("Thread: %d Count: %d\n", val, i);
      //for(j=0;j<10000000;j++);
      uthread_yield ();
    }
  uthread_exit ();
}
