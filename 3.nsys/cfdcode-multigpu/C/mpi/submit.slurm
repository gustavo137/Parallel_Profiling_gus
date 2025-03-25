#!/bin/bash
#SBATCH --time=00:10:00           # Walltime in hh:mm:ss
#SBATCH --nodes=1               # Number of nodes
#SBATCH --ntasks-per-node=4
#SBATCH --cpus-per-task=8
#SBATCH --gres=gpu:4
#SBATCH --job-name=mpi
#SBATCH --exclusive
#SBATCH --partition=boost_usr_prod
#SBATCH -q boost_qos_dbg

module purge
module load nvhpc
module load openmpi

make

rm -rf /tmp/nvidia
ln -s $TMPDIR /tmp/nvidia
nsys profile --trace=mpi -o report-mpi mpirun --map-by socket:PE=8 --rank-by core -np 4 cfd 1000 100
rm -rf /tmp/nvidia

nsys stats report-mpi.nsys-rep > summary.out
