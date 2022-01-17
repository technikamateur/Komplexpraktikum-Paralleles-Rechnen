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
#gcc
#####

#serial version
module load GCC/11.2.0
gcc --version
gcc -O3 -mavx2 -mfma gof_serial.c -o gof.out

#starting loop
for index in "${!sizes[@]}"; do
  for ((k = 0; k < 20; k++)); do
    srun time ./gof.out -s "${sizes[index]}","${sizes[index]}" -R "100" >>gcc_S"${sizes[index]}"_serial.txt 2>&1
  done
done

#simd
gcc -fopenmp -O3 -mavx2 -mfma gof_simd.c -o gof.out

#starting loop
for index in "${!sizes[@]}"; do
  for ((k = 0; k < 20; k++)); do
    srun time ./gof.out -s "${sizes[index]}","${sizes[index]}" -R "100" >>gcc_S"${sizes[index]}"_openmp.txt 2>&1
  done
done

#simd extrem
gcc -fopenmp -std=c99 -O3 -mavx2 -mfma gof_simd_extrem.c -o gof.out simdxorshift128plus.o xorshift128plus.o

#starting loop
for index in "${!sizes[@]}"; do
  for ((k = 0; k < 20; k++)); do
    srun time ./gof.out -s "${sizes[index]}","${sizes[index]}" -R "100" >>gcc_S"${sizes[index]}"_openmp_extrem.txt 2>&1
  done
done

#####
#icc
#####

#serial version
module load intel/2019b
icc --version
icc -O3 -mavx2 -fma gof_serial.c -o gof.out

#starting loop
for index in "${!sizes[@]}"; do
  for ((k = 0; k < 20; k++)); do
    srun time ./gof.out -s "${sizes[index]}","${sizes[index]}" -R "100" >>icc_S"${sizes[index]}"_serial.txt 2>&1
  done
done

#simd
icc -fopenmp -O3 -mavx2 -fma gof_simd.c -o gof_omp.out

#starting loop
for index in "${!sizes[@]}"; do
  for ((k = 0; k < 20; k++)); do
    srun time ./gof_omp.out -s "${sizes[index]}","${sizes[index]}" -R "100" >>icc_S"${sizes[index]}"_openmp.txt 2>&1
  done
done

#simd extrem
icc -fopenmp -std=c99 -O3 -mavx2 -fma gof_simd_extrem.c -o gof_omp_extrem.out simdxorshift128plus.o xorshift128plus.o

#starting loop
for index in "${!sizes[@]}"; do
  for ((k = 0; k < 20; k++)); do
    srun time ./gof_omp_extrem.out -s "${sizes[index]}","${sizes[index]}" -R "100" >>icc_S"${sizes[index]}"_openmp_extrem.txt 2>&1
  done
done

echo "done"
date
