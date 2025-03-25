#!/bin/bash
#SBATCH --nodes=1               # number of nodes
#SBATCH --ntasks-per-node=1     # number of tasks per node
#SBATCH --ntasks-per-socket=1   # number of tasks per socket
#SBATCH --cpus-per-task=1       # number of HW threads per task (equal to OMP_NUM_THREADS*4)
#SBATCH --gres=gpu:1            # this refers to the number of requested gpus per node, and can vary between 1 and 4
#SBATCH --time 00:10:00         # format: HH:MM:SS
#SBATCH --gres=gpu:1
#SBATCH --output=cfd.out
#SBATCH --partition=g100_usr_interactive
#SBATCH --exclusive

module purge
module load /g100_work/PROJECTS/spack/preprod/01/install/0.18/linux-centos8-skylake_avx512/gcc-8.4.1/nvhpc-22.3-jgtetvzbyjbc2o6qpmlcjcjxkp5vi7r5/modulefiles/nvhpc/22.3

make

# uninstrumented run, use time as a reference
./exe 200 50

# instrumented run
rm -rf /tmp/nvidia
ln -s $TMPDIR /tmp/nvidia
nsys profile --backtrace=dwarf  --output=report_serial --force-overwrite=true ./exe 200 50
rm -rf /tmp/nvidia
