.PHONY: all clean

all: ask2-signals

CC = gcc
CFLAGS = -g -Wall -O2

ask2-signals: ask2-signals.o proc-common.o tree.o
	$(CC) $^ -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $<

clean: 
	rm -f *.o ask2-signals
