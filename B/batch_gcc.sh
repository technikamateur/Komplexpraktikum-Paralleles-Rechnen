#!/bin/bash

#SBATCH --nodes=1
#SBATCH --ntasks=1
#SBATCH --ntasks-per-core=1
#SBATCH --cpus-per-task=32
#SBATCH --partition=romeo
#SBATCH --time=04:00:00
#SBATCH --account=p_lv_kp_wise2122
#SBATCH --exclusive
#SBATCH --output=out1.txt
#SBATCH --error=err1.txt
#SBATCH --mem=16000

module purge
module load GCC/11.2.0

gcc --version
gcc -fopenmp gof_parallel.c -o gof.out

export OMP_PLACES=cores
export OMP_PROC_BIND=close
export OMP_DISPLAY_ENV=VERBOSE

# min time
export OMP_THREAD_LIMIT=32
for ((k = 0 ; k < 20 ; k++)); do
  srun ./gof.out -s 128,128 -R 1 >> probe1.txt
done

# setting sizes and threads
sizes=(128 512 2048 8192 32768)
repetitions=(100000 10000 1000 100 10)
threads=(1 2 4 8 16 32)

#starting loop
for index in "${!sizes[@]}"; do
  for j in "${threads[@]}"; do
    export OMP_THREAD_LIMIT=$j
    for ((k = 0 ; k < 20 ; k++)); do
      srun ./gof.out -s "${sizes[index]}","${sizes[index]}" -R "${repetitions[index]}" >> gcc_S"${sizes[index]}"_T"$j".txt 2>&1
    done
  done
done