#!/bin/bash

#SBATCH --nodes=1
#SBATCH --ntasks=1
#SBATCH --ntasks-per-core=1
#SBATCH --cpus-per-task=1
#SBATCH --partition=romeo
#SBATCH --time=01:00:00
#SBATCH --account=p_lv_kp_wise2122
#SBATCH --exclusive
#SBATCH --output=out1.txt
#SBATCH --error=err1.txt
#SBATCH --mem=16000

#setting things up
echo "started"
date
module purge
export OMP_DISPLAY_ENV=VERBOSE
export MKL_DEBUG_CPU_TYPE=5
sizes=(128 512 2048 8192 32768)

#####
#icc
#####

#serial version
module load intel/2019b
icc --version

#starting loop
for index in "${!sizes[@]}"; do
  for ((k = 0; k < 20; k++)); do
    srun time ./gof.out -s "${sizes[index]}","${sizes[index]}" -R "100" >>icc_S"${sizes[index]}"_serial.txt 2>&1
  done
done

#simd

#starting loop
for index in "${!sizes[@]}"; do
  for ((k = 0; k < 20; k++)); do
    srun time ./gof_omp.out -s "${sizes[index]}","${sizes[index]}" -R "100" >>icc_S"${sizes[index]}"_openmp.txt 2>&1
  done
done

#simd extrem

#starting loop
for index in "${!sizes[@]}"; do
  for ((k = 0; k < 20; k++)); do
    srun time ./gof_omp_extrem.out -s "${sizes[index]}","${sizes[index]}" -R "100" >>icc_S"${sizes[index]}"_openmp_extrem.txt 2>&1
  done
done

echo "done"
date
