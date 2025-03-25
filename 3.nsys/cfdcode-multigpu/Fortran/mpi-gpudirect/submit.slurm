#!/bin/bash
#SBATCH --time=00:20:00           # Walltime in hh:mm:ss
#SBATCH --nodes=1               # Number of nodes
#SBATCH --ntasks-per-node=4
#SBATCH --gres=gpu:4
#SBATCH --cpus-per-task=8
#SBATCH --job-name=mpi
#SBATCH --output=cfd.out
#SBATCH --exclusive
#SBATCH --partition=boost_usr_prod

module purge
module load nvhpc
module load openmpi

make

mpirun --map-by socket:PE=8 --rank-by core -np 4 cfd 200 5000

rm -rf /tmp/nvidia
ln -s $TMPDIR /tmp/nvidia
mpirun --map-by socket:PE=8 --rank-by core -np 4 nsys profile -o report-mpi%q{OMPI_COMM_WORLD_RANK} --force-overwrite=true --trace=openacc,mpi,cuda cfd 50 100000 
rm -rf /tmp/nvidia
nsys stats report-mpi1.nsys-rep > summary-1.out
