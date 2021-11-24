#!/bin/bash

#SBATCH --nodes=1
#SBATCH --ntasks=1
#SBATCH --ntasks-per-core=1
#SBATCH --cpus-per-task=32
#SBATCH --partition=romeo
#SBATCH --time=00:10:00
#SBATCH --account=p_lv_kp_wise2122
#SBATCH --exclusive
#SBATCH --output=out.txt
#SBATCH --error=err.txt
#SBATCH --mem=16000

module purge
module load GCC/11.2.0

gcc --version
gcc -fopenmp gof_parallel.c -o gof.out

export OMP_PLACES=cores
export OMP_PROC_BIND=close
export OMP_DISPLAY_ENV=VERBOSE

for ((k = 0 ; k < 19 ; k++)); do
  srun ./gof.out -s 128,128 -R 1
done

sizes=(128 512 2048 8192 32768)

for ((i = 1 ; i <= 32 ; i=i*2)); do
  export OMP_THREAD_LIMIT=$i
  for ((j = 0 ; j <= 4 ; j++)); do
    power=10**$j
    repetitions=100000/$power
    for ((k = 0 ; k < 19 ; k++)); do
      srun time ./gof.out -s "${sizes[$j]}","${sizes[$j]}" -R "$repetitions" >> gcc_T"$i"_R"$j".txt 2>&1
    done
  done
done
