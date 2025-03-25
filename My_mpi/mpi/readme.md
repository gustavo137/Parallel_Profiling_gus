# Compile 
We need to load the module mpi ... (using module load mpi...)

then we use:
~~~
mpicc main.c -o main.x
~~~
to run a program .cpp or .c that uses libraryes of c and cpp for example "#include<mpi.h>"and "#include<iostream>" 
we use:
~~~
mpic++ main.c -o main.x
~~~
this mpic++ runs with .c and .cpp
to run we use:
~~~
mpirun -np NUMBER ./main.x
~~~
NUMBER is the number of prcessor that we want to use, in this computer Lenovo we only can use NUMBER=2.

To simulate more processors on your computer using MPI, you can use the --oversubscribe option 
when running your program with mpirun. This tells MPI to allow more processes than your computer has
 in terms of physical or logical cores, allowing you to run more processes than your hardware supports
 for testing or development purposes.
~~~
mpirun --oversubscribe -np 4 ./hello.x
~~~
