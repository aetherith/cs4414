# Thomas Foulds (tcf9bj)
# Hong Moon (hsm5xw)
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

add:
	git add -A

commit:
	git commit

push:
	git push origin master

.SUFFIXES: .o .c

shell.o: shell.c shell.h
