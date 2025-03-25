#!/bin/bash
#SBATCH --time=00:20:00           # Walltime in hh:mm:ss
#SBATCH --nodes=1               # Number of nodes
#SBATCH --ntasks-per-node=20
#SBATCH --cpus-per-task=1
#SBATCH --exclusive
#SBATCH --job-name=scorep
#SBATCH --partition=g100_usr_dbg

module purge
module load intel/oneapi-2022--binary
module load intelmpi/oneapi-2022--binary
source /cineca/prod/opt/compilers/intel/oneapi-2022/binary/setvars.sh

make

srun -n 20 --cpus-per-task=1 --cpu-bind=cores ./exe 10 10000
