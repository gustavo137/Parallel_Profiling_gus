#!/bin/bash
  
# sbatch settings
#SBATCH --job-name=roofline
#SBATCH --time=0:30:00
#SBATCH --nodes=1
#SBATCH --ntasks-per-node=1
#SBATCH --partition=g100_usr_dbg
##SBATCH --qos=g100_qos_dbg
#SBATCH --mem=0
#SBATCH --cpus-per-task=48
#SBATCH --exclusive
#SBATCH --output=%x_%j.out
#SBATCH --error=%x_%j.err


module load intel-oneapi-advisor/2023.2.0-6eiojq7
module load gcc/12.2.0

export OMP_NUM_THREADS=${SLURM_CPUS_PER_TASK:-1}
export LC_ALL="C" 

mkdir -p bin
cd bin
rm -r *
cd ../src
make clean
make 
cd ../bin

srun advisor --collect=survey       --project-dir=advisor_proj_c  -- ./main_c.x
srun advisor --collect=tripcounts   --project-dir=advisor_proj_c  --flop  --stacks --enable-cache-simulation  --  ./main_c.x
srun advisor --report=roofline      --project-dir=advisor_proj_c  #--with-stack --memory-level=L1_L2_L3_DRAM
srun advisor --report=survey        --project-dir=advisor_proj_c 


#srun advisor --collect=survey       --project-dir=advisor_proj_cpp  -- ./main_cpp.x
#srun advisor --collect=tripcounts   --project-dir=advisor_proj_cpp  --flop  --stacks --enable-cache-simulation  --  ./main_cpp.x
#srun advisor --report=roofline      --project-dir=advisor_proj_cpp  #--with-stack  #--memory-level=L1_L2_L3_DRAM
#srun advisor --report=survey        --project-dir=advisor_proj_cpp 


