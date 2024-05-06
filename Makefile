CFLAGS=-std=c11 -g -static
SRCS=$(wildcard *.c)
OBJS=$(SRCS:.c=.o)
TESTDIR=testdir/
TESTSRCS=$(wildcard ${TESTDIR}*.c)

9cc: $(OBJS)
	$(CC) -o 9cc $(OBJS) $(LDFLAGS)
prepare:
	./9cc ${TESTDIR}check.txt > tmp.s
	$(CC) -c $(TESTSRCS)

$(OBJS): 9cc.h

test: 9cc prepare
	$(CC) -o check tmp.s act.o expc.o
	./check

clean:
	rm -f 9cc *.o *~ tmp* input* check*

.PHONY: test clean
