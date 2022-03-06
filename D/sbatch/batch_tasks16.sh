#!/bin/bash

#SBATCH --nodes=1
#SBATCH --ntasks-per-node=16
#SBATCH --cpus-per-task=1
#SBATCH --partition=romeo
#SBATCH --time=00:50:00
#SBATCH --account=p_lv_kp_wise2122
#SBATCH --output=out16.txt
#SBATCH --error=err16.txt
#SBATCH --mem-per-cpu=1972
#SBATCH --exclusive

module purge
module load OpenMPI/4.1.2-GCC-11.2.0

mpicc --version
mpirun --version
mpicc -O3 -mavx2 -mfma gof_mpi_chessboard.c -o gof_mpi_chessboard.o -lm

echo "started"
date

sizes=(2048 8192 32768)
repetitions=(10000 1000 100)

#starting loop
for index in "${!sizes[@]}"; do
  for ((k = 0; k < 20; k++)); do
    time srun --cpu_bind=cores --distribution=block:block ./gof_mpi_chessboard.o -s "${sizes[index]}","${sizes[index]}" -R "${repetitions[index]}" >> Tasks16_S"${sizes[index]}".txt 2>&1
  done
done

echo "done"
date
