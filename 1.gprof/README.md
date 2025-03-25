# PROFILING WITH GPROF 

Test case adapted from https://github.com/davidhenty/cfd/blob/master/F-SER/cfd.f90

- enter an interactive session

- enter the cfd/src folder and add profiling option -pg to the Makefile

- compile with make

- run the instrumented binary (provide input parameters 128 30) and postprocess the binary profile 


1. Which is the most time-consuming routine ?

2. How much time is spent in MAIN_ exclusively ?

3. Are there routines contributing to _MAIN with negligible exclusive time ?

4. Are there routines with more than one calling path ?

5. Repeat by increasing the optimization flag. Are there changes in the flat summary ?

