# Thomas Foulds (tcf9bj)
# Amanda Ray (ajr2fu)
# Sami Fekadu (sf5fw)
# 04/14/13
# CS4414
# Makefile for Assignment 6

CFLAGS=-Wall
CC=gcc $(CFLAGS)
OFILES=recover.o

default: $(OFILES)
	$(CC) -g -o recover $(OFILES)

clean:
	rm -f *~ *.o recover

test: clean default clearscr
	./recover 10578 1231 datafile.le

test-mem: clean default clearscr valgrind

clearscr:
	clear

valgrind:
	valgrind --tool=memcheck --leak-check=yes ./recover 10578 1231 datafile.le

.SUFFIXES: .o .c

recover.o: recover.c recover.h
