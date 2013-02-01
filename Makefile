# Thomas Foulds
# tcf9bj
# 02/01/13
# CS4414
# Makefile for Assignment 2

CFLAGS=-Wall
CC=gcc $(CFLAGS)
OFILES=shell.o

default: $(OFILES)
	$(CC) -g -o lab2sh $(OFILES)

test: clean default clearscr valgrind

valgrind:
	valgrind --tool=memcheck --leak-check=yes ./lab2sh

clearscr:
	clear

clean:
	rm -f *o *~ lab2sh

.SUFFIXES: .o .c

shell.o: shell.c shell.h
