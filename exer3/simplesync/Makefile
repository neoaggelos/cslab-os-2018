CC = gcc
CFLAGS = -Wall -pthread

all: simplesync-mutex simplesync-atomic

simplesync-mutex: simplesync-mutex.o
	$(CC) $(CFLAGS) $< -o $@

simplesync-atomic: simplesync-atomic.o
	$(CC) $(CFLAGS) $< -o $@

simplesync-mutex.o: simplesync.c
	$(CC) -c -DSYNC_MUTEX $< -o $@

simplesync-atomic.o: simplesync.c
	$(CC) -c -DSYNC_ATOMIC $< -o $@

clean: 
	rm -rf *.o simplesync-mutex simplesync-atomic
