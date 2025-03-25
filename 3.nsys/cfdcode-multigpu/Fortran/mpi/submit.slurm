#!/bin/bash
#SBATCH --time=01:00:00           # Walltime in hh:mm:ss
#SBATCH --nodes=1               # Number of nodes
#SBATCH --ntasks-per-node=4
#SBATCH --cpus-per-task=8
#SBATCH --job-name=mpi
#SBATCH --output=cfd.out
#SBATCH --exclusive
#SBATCH --partition=boost_usr_prod

module purge
module load nvhpc
module load openmpi

mpirun --map-by socket:PE=8 --rank-by core -np 4 cfd 200 100

make

rm -rf /tmp/nvidia
ln -s $TMPDIR /tmp/nvidia
nsys profile -o report-mpi --force-overwrite=true --backtrace=dwarf --trace=mpi mpirun --map-by socket:PE=8 --rank-by core -np 4 cfd 50 100
rm -rf /tmp/nvidia

nsys stats report-mpi.nsys-rep > summary.out
