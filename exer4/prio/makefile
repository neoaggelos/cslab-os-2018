all: scheduler-shell prog shell

%.o: %.c
	gcc -Wall -c $< -o $@

scheduler-shell: scheduler-shell.o plist.o proc-common.o

shell: shell.o

prog: prog.o proc-common.o

clean:
	rm -rf *.o scheduler-shell prog shell