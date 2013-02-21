# Thomas Foulds (tcf9bj)
# 02/18/13
# CS4414
# Makefile for Assignment 3

CFLAGS=-Wall
CC=gcc $(CFLAGS)
OFILES=list_item_t.o list_t.o thread_t.o uthread.o
SOFILES=test-simple.o $(OFILES)
MOFILES=test-multi-threads.o $(OFILES)

default: $(SOFILES)
	$(CC) -g -o lab3s $(SOFILES)

multithread:
	$(CC) -g -o lab3m $(MOFILES)

threadpkg1:
	$(CC) -g -c -o threadpkg1.o uthread.c

test: clean default clearscr valgrind

valgrind:
	valgrind --tool=memcheck --leak-check=yes ./lab3s

clearscr:
	clear

clean:
	rm -f *o *~ lab3s lab3m

add:
	git add -A

commit:
	git commit

pushm:
	git push origin master

.SUFFIXES: .o .c

list_item_t.o: list_item_t.c list_item_t.h
list_t.o: list_t.c list_t.h list_item_t.h
test-create.o: test-create.c uthread.h list_t.h list_item_t.h thread_t.h
test-multi-threads.o: test-multi-threads.c uthread.h list_t.h \
 list_item_t.h thread_t.h
test-simple.o: test-simple.c uthread.h list_t.h list_item_t.h thread_t.h
thread_t.o: thread_t.c thread_t.h list_item_t.h
uthread.o: uthread.c uthread.h list_t.h list_item_t.h thread_t.h
