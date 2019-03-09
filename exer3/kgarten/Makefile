CC = gcc
CFLAGS = -O2 -pthread -Wall

%.o: %.c
	$(CC) -c $(CFLAGS) $< -o $@

kgarten: kgarten.o
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm *.o kgarten
