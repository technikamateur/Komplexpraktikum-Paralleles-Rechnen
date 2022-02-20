#!/bin/bash

#SBATCH --nodes=1
#SBATCH --ntasks-per-node=128
#SBATCH --cpus-per-task=1
#SBATCH --partition=romeo
#SBATCH --time=01:00:00
#SBATCH --account=p_lv_kp_wise2122
#SBATCH --output=out128.txt
#SBATCH --error=err128.txt
#SBATCH --mem-per-cpu=1972
#SBATCH --exclusive

module purge
module load OpenMPI/4.1.2-GCC-11.2.0

mpicc --version
mpirun --version
mpicc -O3 -mavx2 -mfma gof_mpi_chessboard.c -o gof_mpi_chessboard.o -lm

echo "started"
date

sizes=(2048 8192 32768 131072)
repetitions=(10000 1000 100 10)

#starting loop
for index in "${!sizes[@]}"; do
  for ((k = 0; k < 20; k++)); do
    time srun --cpu_bind=cores --distribution=block:block ./gof_mpi_chessboard.o -s "${sizes[index]}","${sizes[index]}" -R "${repetitions[index]}" >> Tasks128_S"${sizes[index]}".txt 2>&1
  done
done

echo "done"
date
