#!/bin/bash
#SBATCH --time=00:10:00           # Walltime in hh:mm:ss
#SBATCH --nodes=1               # Number of nodes
#SBATCH --ntasks-per-node=4
#SBATCH --cpus-per-task=8
#SBATCH --job-name=mpi
#SBATCH --output=cfd.out
#SBATCH --exclusive
#SBATCH --partition=boost_usr_prod
#SBATCH -q boost_qos_dbg

module purge
module load nvhpc/23.11
module load openmpi/4.1.6--nvhpc--23.11

make

rm -rf /tmp/nvidia
ln -s $TMPDIR /tmp/nvidia
nsys profile -o report-mpi --force-overwrite=true --backtrace=dwarf --trace=mpi mpirun --map-by socket:PE=8 --rank-by core -np 4 exe 50 100
rm -rf /tmp/nvidia

nsys stats report-mpi.nsys-rep > summary.out
