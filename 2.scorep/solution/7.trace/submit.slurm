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

export SCOREP_ENABLE_PROFILING=ON
export SCOREP_EXPERIMENT_DIRECTORY=scorep_run
export SCOREP_TOTAL_MEMORY=2GB
export SCOREP_METRIC_PAPI=PAPI_TOT_INS,PAPI_TOT_CYC

pool=( 20 )

for i in "${pool[@]}"
do
        scalasca -analyze -f filter.file -t srun --cpu-bind=cores --cpus-per-task=1 -n $i exe 10 10000
	echo "done"
	wait
done

