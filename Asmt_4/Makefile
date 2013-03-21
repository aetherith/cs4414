ifeq (, $(shell which clang))
	CC		= gcc
else
	CC		= clang
endif
LD			= $(CC)
LDFLAGS		=
CFLAGS		= -Wno-deprecated-declarations
ifeq ($(shell uname), Darwin) # Apple
	# ucontext conflicts with LLVM when optimization is enabled
	CFLAGS	+= -O0
else
	CFLAGS	+= -O3
endif
CPPFLAGS	=

BIN			= uthread_test
OBJS		= uthread_test.o uthread.o

ARCHIVE		= lab3-rev-`git rev-parse HEAD | cut -c 1-8`

all: uthread.o
	cp uthread.o threadpkg1.o

$(BIN): $(OBJS)
	$(LD) $(LDFLAGS) -o $(BIN) $(OBJS)

%.o: %.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -c -o $@ $<

clean:
	rm -f $(BIN)
	rm -f *.o
	rm -f *.tar.gz

run: $(BIN)
	./uthread_test

memcheck: $(BIN)
	valgrind ./uthread_test

archive: clean
	git archive master > $(ARCHIVE).tar
	git log > CHANGELOG
	tar rvf $(ARCHIVE).tar CHANGELOG
	gzip $(ARCHIVE).tar

