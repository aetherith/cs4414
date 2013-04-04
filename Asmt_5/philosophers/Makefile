ifeq (, $(shell which clang))
	CC		= gcc
	CXX		= g++
else
	CC		= clang
	CXX		= clang++
endif
LD			= $(CC)
LDFLAGS		= -lpthread
CFLAGS		= -std=c99 -g
CPPFLAGS	=

BIN			= philosophers philosophers_stats

ARCHIVE		= lab5-rev-`git rev-parse HEAD | cut -c 1-8`

all: $(BIN)

philosophers: philosophers.o
	$(LD) $(LDFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -c -o $@ $<

philosophers_stats: philosophers_stats.cc
	$(CXX) -O3 -o $@ $<

clean:
	rm -f $(BIN)
	rm -f *.o
	rm -f *.tar.gz

run: $(BIN)
	./philosophers 5 0.1 0 | ./philosophers_stats

memcheck: $(BIN)
	valgrind ./philosophers 5 0.1 0
