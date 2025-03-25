#!/bin/bash

make clean -f Makefile*
sed 's/-O2/-O2 -pg/' Makefile > Makefile_gprof
make -f Makefile_gprof

srun -n 1 exe 128 30
gprof exe gmon.out > profile.txt
rm gmon.out
rm velocity* colourmap*
