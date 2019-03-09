CC = gcc
CFLAGS = -Wall -pthread
LIBS = -lm

all: mandel mandel2

mandel: mandel-lib.o mandel.o
	$(CC) $(CFLAGS) -o mandel mandel-lib.o mandel.o $(LIBS)

mandel2: mandel-lib.o mandel2.o
	$(CC) $(CFLAGS) -o mandel2 mandel-lib.o mandel2.o $(LIBS)

mandel-lib.o: mandel-lib.h mandel-lib.c
	$(CC) $(CFLAGS) -c -o mandel-lib.o mandel-lib.c

mandel.o: mandel.c
	$(CC) $(CFLAGS) -c -o mandel.o mandel.c $(LIBS)

mandel2.o: mandel2.c
	$(CC) $(CFLAGS) -c -o mandel2.o mandel2.c $(LIBS)

clean:
	rm -rf *.o mandel mandel2
