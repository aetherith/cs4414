# Thomas Foulds
# tcf9bj
# 01/22/13
# CS4414
# Makefile for Assignment 1

CFLAGS=-Wall
CC=gcc $(CFLAGS)
OFILES=harness.o list_item_t.o list_t.o

default: $(OFILES)
	$(CC) -g -o lab1 $(OFILES)

clean:
	rm -f *.o *~ lab1

.SUFFIXES: .o .c

list_item_t.o: list_item_t.c list_item_t.h
list_t.o: list_t.c list_t.h list_item_t.c list_item_t.h
harness.o: list_t.c list_t.h list_item_t.c list_item_t.h
