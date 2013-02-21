/* Code to test the uthread library */

#include <stdio.h>
#include <unistd.h>
#include "uthread.h"

void threadFunc(int);

int main()
{
	int i=0;

	uthread_init();

	uthread_create(threadFunc, 1, 0);
	uthread_create(threadFunc, 2, 0);
	uthread_create(threadFunc, 3, 0);
	uthread_create(threadFunc, 4, 0);

	for(i=0;i < 10;i++)
	{
          printf("This is the main function: %i\n", i);
		uthread_yield();
	}
	printf("I am main and I am exiting\n");
	uthread_exit();
	return 0;
}

/* Lame user thread */
void threadFunc(int val)
{
	int i,j;

	for(i=0;i<5;i++)
	{
		printf("Thread: %d Count: %d\n",val,i);
		for(j=0;j<10000000;j++);
		uthread_yield();
	}
	uthread_exit();
}
