CC=gcc
CFLAGS=-Wall
BUILDDIR=./build/

all: shell prog shell2

shell:
	$(CC) shell.c -o $(BUILDDIR)shell

prog:
	$(CC) prog.c -o $(BUILDDIR)prog

shell2:
	$(CC) shell2.c -o $(BUILDDIR)shell2

clean:
	rm $(BUILDDIR)*
