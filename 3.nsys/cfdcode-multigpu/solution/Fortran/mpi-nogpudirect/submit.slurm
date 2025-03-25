#!/bin/bash
#SBATCH --time=00:10:00           # Walltime in hh:mm:ss
#SBATCH --nodes=1               # Number of nodes
#SBATCH --ntasks-per-node=4
#SBATCH --cpus-per-task=8
#SBATCH --job-name=mpi
#SBATCH --output=cfd.out
#SBATCH --gres=gpu:4
#SBATCH --exclusive
#SBATCH --partition=boost_usr_prod
#SBATCH -q boost_qos_dbg

module purge
module load nvhpc/23.11
module load openmpi/4.1.6--nvhpc--23.11

make

export TMPDIR=/dev/shm
ln -s $TMPDIR /tmp/nvidia
nsys profile --trace=openacc,mpi,cuda,nvtx mpirun --map-by socket:PE=8 --rank-by core -np 4 cfd 50 10000 
rm -rf /tmp/nvidia

