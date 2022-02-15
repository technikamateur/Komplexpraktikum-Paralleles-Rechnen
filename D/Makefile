CC=mpicc
CFLAGS=-O3 -mavx2 -mfma
HEADERS=simdxorshift128plus.h
RM=rm -r

all: gof_mpi.o gof_mpi_chessboard.o

clean:
	$(RM) *.o

gof_mpi_chessboard.o: gof_mpi_chessboard.c
	$(CC) $(CFLAGS) $< -o $@ -lm

gof_mpi.o: gof_mpi.c $(HEADERS) simdxorshift128plus.o Makefile
	$(CC) $(CFLAGS) $< -o $@ simdxorshift128plus.o

simdxorshift128plus.o: simdxorshift128plus.c $(HEADERS) Makefile
	gcc -fPIC -std=c99 -O3 -mavx2  -march=native -Wall -Wextra -pedantic -Wshadow -Iinclude -c $< -o $@

.PHONY: clean
