.PHONY: all clean

all: ask2-tree

CC = gcc
CFLAGS = -g -Wall -O2


ask2-tree: ask2-tree.o proc-common.o tree.o
	$(CC) $^ -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $<

clean: 
	rm -f *.o ask2-tree
