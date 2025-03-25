#!/bin/bash
#SBATCH --nodes=1               # number of nodes
#SBATCH --ntasks-per-node=1     # number of tasks per node
#SBATCH --cpus-per-task=1       # number of HW threads per task (equal to OMP_NUM_THREADS*4)
#SBATCH --gres=gpu:1            # this refers to the number of requested gpus per node, and can vary between 1 and 4
#SBATCH --time 00:10:00         # format: HH:MM:SS
#SBATCH --output=mand.out
#SBATCH -p g100_usr_interactive
#SBATCH --exclusive

module purge
module load /g100_work/PROJECTS/spack/preprod/01/install/0.18/linux-centos8-skylake_avx512/gcc-8.4.1/nvhpc-22.3-jgtetvzbyjbc2o6qpmlcjcjxkp5vi7r5/modulefiles/nvhpc/22.3

make 

./exe

rm -rf /tmp/nvidia
ln -s $TMPDIR /tmp/nvidia
nsys profile --trace=openacc --output=report-mandelbrot --force-overwrite=true ./exe
rm -rf /tmp/nvidia

