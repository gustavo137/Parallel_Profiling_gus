#!/bin/bash
#SBATCH --time=00:20:00           # Walltime in hh:mm:ss
#SBATCH --nodes=1               # Number of nodes
#SBATCH --exclusive
#SBATCH --job-name=scorep
#SBATCH --partition=g100_usr_dbg

module purge
source modules.sh
module list

make

pool=( 20 )

for i in "${pool[@]}"
do
        scalasca -analyze srun --cpu-bind=cores --cpus-per-task=1 -n $i exe 10 10000
	echo "done"
	wait
done

