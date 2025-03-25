#!/bin/bash
module load profile/candidate
module load spack/preprod-0.18.1-01
module load scorep
module load scalasca
module load intel-oneapi-compilers

src=jacobi-mpi-sendrecv.f90

scorep --mpp=mpi --user mpiifort -fpp  $src -o exe

