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
#SBATCH -q boost_qos_dbg

module purge
module load nvhpc/23.11
module load openmpi/4.1.6--nvhpc--23.11

make

export TMPDIR=/dev/shm
ln -s $TMPDIR /tmp/nvidia
nsys profile --trace=openacc,mpi,cuda mpirun --map-by socket:PE=8 --rank-by core -np 4 exe 50 10000 
# to use with more nodes (for this example use 2 nodes) we need this way to run
#mpirun --map-by socket:PE=8 --rank-by core -np 4 nsys profile --trace=openacc,mpi,cuda -o r_2mnodes exe 50 10000
#mpirun --map-by socket:PE=8 --rank-by core nsys profile --trace=openacc,mpi,cuda -o r_2v2nodes --nic-metrics=true exe 50 10000
rm -rf /tmp/nvidia

