#!/bin/bash
#SBATCH --time=00:20:00           # Walltime in hh:mm:ss
#SBATCH --nodes=1               # Number of nodes
#SBATCH --exclusive
#SBATCH --job-name=scorep
#SBATCH --partition=g100_usr_dbg
#SBATCH --cpus-per-task=1

module purge
source modules.sh
module list

make

export SCOREP_EXPERIMENT_DIRECTORY=<add-here>
export SCOREP_METRIC_PAPI=<add-here>


pool=( 20 )

for i in "${pool[@]}"
do
        scalasca -analyze <add-here> srun --cpu-bind=cores --cpus-per-task=$SLURM_CPUS_PER_TASK -n $i exe 10 10000
	echo "done"
	wait
done

