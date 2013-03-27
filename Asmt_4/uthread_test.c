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
  //uthread_create (threadFunc, 5, 5);
  //uthread_create (threadFunc, 6, 3);
  //uthread_create (threadFunc, 7, 3);
  //uthread_create (threadFunc, 8, 2);

  // uncomment to test uthread_exit() on main earlier than others
  //uthread_exit();

  while( 1 ) printf("Waiting for interrupt.\n");

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

  //if( val%2 == 0 ) uthread_exit();

  for (i = 0; i < 5; i++)
    {
      printf ("Thread: %d Count: %d\n", val, i);
      for(j=0;j<10000000;j++)
        {
          if( j%1000000 == 0 ) printf("%i*", val);
        }
      printf("\n");
      uthread_yield ();
    }
  uthread_exit ();
}
