#!/bin/bash
#SBATCH -A ICT24_MHPC
#SBATCH --job-name=exampleSpeciaCastBytes # Job name
#SBATCH --mail-type=END,FAIL              # Mail events (NONE, BEGIN, END, FAIL, ALL)
#SBATCH --mail-user=gparedes@ictp.it      # Where to send mail
#SBATCH -N 1                               # Number of nodes
#SBATCH --ntasks-per-node=1               # Run on a single CPU
#SBATCH --time=00:05:00                   # Time limit hrs:min:sec
#SBATCH -p boost_usr_prod      # for smaller works use --qos=boost_qos_dbg , boost is dedicated to works with GPU


echo "Running script timer mpi"
module load openmpi/4.1.6--gcc--12.2.0 
mpic++ exampleSpeciaCastBytes.cpp -o exampleSpeciaCastBytes.x -I.

mpirun -np 2 ./exampleSpeciaCastBytes.x

