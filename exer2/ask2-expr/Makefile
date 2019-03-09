.PHONY: all clean

all: ask2-expr

CC = gcc
CFLAGS = -g -Wall -O2

ask2-expr: ask2-expr.o proc-common.o tree.o
	$(CC) $^ -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $<

clean: 
	rm -f *.o ask2-expr
