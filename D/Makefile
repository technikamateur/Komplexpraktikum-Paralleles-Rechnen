CC=mpicc
CFLAGS=-O3 -mavx2 -mfma
RM=rm -r

all: gof_mpi_chessboard.o

clean:
	$(RM) *.o

gof_mpi_chessboard.o: gof_mpi_chessboard.c
	$(CC) $(CFLAGS) $< -o $@ -lm

.PHONY: clean
