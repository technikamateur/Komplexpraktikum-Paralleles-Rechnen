CC=gcc
CFLAGS=-fopenmp
RM=rm -r

all: gof.o

clean:
	$(RM) *.o

gof.o: gof_parallel.c Makefile
	$(CC) $(CFLAGS) $< -o $@

.PHONY: clean
