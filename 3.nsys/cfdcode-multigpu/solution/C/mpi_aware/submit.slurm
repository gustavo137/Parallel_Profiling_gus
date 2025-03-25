#!/bin/bash
#SBATCH --nodes=1               # number of nodes
#SBATCH --ntasks-per-node=4     # number of tasks per node
#SBATCH --cpus-per-task=8       # number of HW threads per task (equal to OMP_NUM_THREADS*4)
#SBATCH --gres=gpu:4            # this refers to the number of requested gpus per node, and can vary between 1 and 4
#SBATCH --hint=nomultithread
#SBATCH --time 00:10:00         # format: HH:MM:SS
#SBATCH -p boost_usr_prod
#SBATCH -q boost_qos_dbg

module purge
module load nvhpc/24.3
module load openmpi/4.1.6--nvhpc--24.3 

make

export TMPDIR=/dev/shm
ln -s $TMPDIR /tmp/nvidia
/leonardo/prod/spack/5.2/install/0.21/linux-rhel8-icelake/gcc-12.2.0/nvhpc-24.9-mdaafnrykclsssc7znojbiytdf4hg346/Linux_x86_64/24.9/compilers/bin/nsys profile --trace=cuda,openacc,mpi -o report_acc --force-overwrite=true mpirun  ./cfd 5 2
rm -rf /tmp/nvidia


