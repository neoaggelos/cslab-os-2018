all: scheduler prog

%.o: %.c
	gcc -Wall -c $< -o $@

scheduler: scheduler.o plist.o proc-common.o

prog: prog.o proc-common.o

clean:
	rm -rf *.o scheduler prog