# Thomas Foulds (tcf9bj)
# Amanda Ray (ajr2fu)
# 03/28/13
# CS4414
# Makefile for Assignment 4

CFLAGS=-Wall
CC=gcc $(CFLAGS)
OFILES=uthread.o
threadpkg2:
	$(CC) -g -c -o threadpkg2.o uthread.c

clean:
	rm -f *~ *.o

.SUFFIXES: .o .c

uthread.o: uthread.c uthread.h
