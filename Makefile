CFLAGS=-std=c11 -g -static
SRCSDIR=src/
SRCS=$(wildcard ${SRCSDIR}*.c)
OBJS=$(SRCS:.c=.o)
TESTDIR=test/
TESTSRCS=$(wildcard ${TESTDIR}*.c)

9cc: $(OBJS)
	$(CC) -o 9cc $(OBJS) $(LDFLAGS)

$(OBJS): ${SRCSDIR}9cc.h

prepare: 9cc
	./9cc ${TESTDIR}check.txt > tmp.s
	$(CC) -c $(TESTSRCS)
	$(CC) -o check tmp.s act.o expc.o print.o

test:
	./check

clean:
	rm -f 9cc *.o *~ tmp* input* check*

.PHONY: test clean
