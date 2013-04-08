# Thomas Foulds (tcf9bj)
# Amanda Ray (ajr2fu)
# Sami Fekadu (sf5fw)
# CS4414
# 04/08/13
# Assignment 5
# Dining Philosophers

CC = gcc
CXX = g++
LD			= $(CC)
LDFLAGS		= -lpthread
CFLAGS		= -std=c99 -g
CPPFLAGS	=

BIN			= philosophers philosophers_stats

default: philosophers.o
	$(LD) $(LDFLAGS) -o philosophers $^

%.o: %.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -c -o $@ $<

philosophers_stats: philosophers_stats.cc
	$(CXX) -O3 -o $@ $<

clean:
	rm -f $(BIN)
	rm -f *.o
