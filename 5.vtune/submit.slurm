#!/bin/bash
#SBATCH --time=00:10:00           # Walltime in hh:mm:ss
#SBATCH --nodes=1               # Number of nodes
#SBATCH --ntasks-per-node=1
#SBATCH --cpus-per-task=12
#SBATCH --account=cin_staff
#SBATCH --job-name=test
#SBATCH --exclusive
#SBATCH --gres=sysfs --exclusive
#SBATCH --partition=g100_usr_prod

module purge
module load autoload
module load intel/oneapi-2022--binary
source /cineca/prod/opt/compilers/intel/oneapi-2022/binary/setvars.sh

export OMP_NUM_THREADS=12

ifort -g -qopenmp -O2 -parallel-source-info=2 prime_static.f90 -o prime_static.exe
wait
vtune -collect hpc-performance -knob sampling-interval=2 -r results-static -- ./prime_static.exe


