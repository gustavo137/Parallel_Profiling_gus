#!/bin/bash
#SBATCH --time=00:10:00           # Walltime in hh:mm:ss
#SBATCH --nodes=1               # Number of nodes
#SBATCH --exclusive
#SBATCH --job-name=scorep
#SBATCH --partition=g100_usr_dbg

module purge
module load profile/candidate
module load spack/preprod-0.18.1-01
module load scorep
module load scalasca
module load intel-oneapi-compilers

source compile.sh

export SCOREP_TOTAL_MEMORY=400MB

pool=( 12 )
exe=jacobi-block-instr

for i in "${pool[@]}"
do
	export cpus=$((48/$i))
	echo "$cpus"
        scalasca -analyze srun --cpu-bind=cores --cpus-per-task=$cpus -n $i exe 500 1000  
	wait
done
