Exercises
=========

1. Performance engineering workflow with nsys
---------------------------------------------

In this exercise, we implement a performance engineering workflow assisted by NSight Systems. Starting from the serial code `here <https://gitlab.hpc.cineca.it/nways_accelerated_programming/basictoadvancedopenx/-/tree/main/exercises/cfdcode-onegpu/Fortran?ref_type=heads>`_, we offload incrementally the application with OpenACC directives and generate reports to understand the bottlenecks.

The code implements a typical CFD loop based on the following steps

* A jacobi step in ``jacobi_acc`` routine

.. code-block:: console

   do j = 1, n
      do i = 1, m
          psinew(i, j) = 0.25d0*(psi(i+1, j) + psi(i-1, j) + &
                             psi(i, j+1) + psi(i, j-1)     )
      end do
   end do

* reduction to compute the error in ``deltasq`` routine

.. code-block:: console

   deltasq = 0.d0
   do j = 1, n
     do i = 1, m
       deltasq = deltasq + (new(i,j)-old(i,j))**2
     end do
    end do

* backup of the last psi in the main loop.

.. code-block:: console

   !  Copy back
   do j=1,m
     do i=1,n
       psi(i,j) = psitmp(i,j)
     end do
   end do


The steps are repeated until convergence or reaching a maximum number of steps.

At the end of the simulation some routines handle the data postprocessing for visualization.

.. warning::

   Allocating jobs in exclusive mode is highly suggested


The code takes two values as input: (i) scale factor [200] for the grid and (ii) step number [50]

2.1 As a first step we generate the reference run in ``./serial/`` folder.

* 2.1.1 Load the modules for the NVHPC compiler ( check the jobfile )
      
.. code-block:: console
    
   module load nvhpc

* 2.1.2 Compile with ``make``. 
* 2.1.3 Submit the jobfile to genere the reference run.
* 2.1.4 How much time does the code take in the serial version? (check slurm logfile)

.. warning::

   On Leonardo ``--backtrace=dwarf`` is needed to sample the CPU


2.2 Instrument the application in a new ``./nvtx/`` folder

Nvtx markers are added in the source code for (i) jacobi, (ii) deltasq, (iii) main loop and (iv) the write phase at the end of the simulation.

* 2.2.2 Submit the job script to generate the profile ( ``*.nsys-rep`` )
* 2.3.3 Open with the GUI.  Which routines take longer? Which part of the trace is not interesting for porting? 
  
2.3 Modify the jobfile in order to filter the application on ``main_loop`` only.

2.4 First Offload in ``./first_offload/`` 

This version of the code has been parallelized withouting managing data movement. 

* 2.4.1 Submit the job script to generate the report.
* 2.4.2 Check the time to execute the main loop. Do you observe a speed up?
* 2.4.3 Open the report and look at the data movement section in the GPU panel. Which is the ratio between memory and kernels? Can you identify in the timeline what is happening inside one ``jacobi`` step?
* 2.4.4 What are gaps in the CUDA HW panel?
* 2.4.5 Select one "Compute Construct" in the OpenACC row. How many gangs and workers are used to offload the OpenACC construct? 
* 2.4.6 How much time does the CPU spend waiting? How long does the kernel actually take on the GPU? Are CPU and GPUs synchronous or not? 
* 2.4.7 Generate a statistical summary to see how much time is spent on average moving the data and their largest size.

2.5 Improved offload in ``./improve_offload``

This version of the code handles data movement explicitly.

* 2.5.1 Submit the job file to run the code. Check time to solution and time per iteration. 
* 2.5.2 Do you observe a speedup? Which is the ratio between kernel and memory percentage now?
* 2.5.3 Open the profile and check the timelineview. How did it change with respect to the previous one?
* 2.5.4 Check from the statistical summary the time spent in H2D, D2H and kernel computation


SPOILER! Solution follows

.. code-block:: console

   $ grep "Each individual" */cfd.out
   3.nvtx-filt/cfd.out: Each individual iteration took  0.1478     seconds
   4.no-acc-data/cfd.out: Each individual iteration took  0.2134     seconds
   5.acc-data/cfd.out: Each individual iteration took  0.8222E-02 seconds

2. Mandelbrot
------------

In this exercise, each pixel of an image is processed by a mandelbrot routine, which is called from within a double loop. The routine is offloaded to GPUs with OpenACC, initially by launching one kernel where threads execute their own istance of mandelbrot.

* 3.1 Profile the initial implementation in ``noblock/`` with nsys profile; extract time spent in kernel computation and data movement. How much time is takes to compute the whole kernel, including moving data?
* 3.2 Profile the block implementation in ``block/``; this version uses the block technique, and does an update D2H for each block rather than a unique movement of data.
* 3.3 ``streams/`` uses an optimized version based on multiple streams. Run and profile the simulation by submitting the job script. Does time to execute the kernel decrease? Visualize the trace. How many streams are used? Which operations are overlapped?  


3. Multi-GPU 
------------

The code can be found `here <https://gitlab.hpc.cineca.it/nways_accelerated_programming/basictoadvancedopenx/-/tree/main/exercises/cfdcode-multigpu?ref_type=heads>`_ . It is the distributed version of the previous one. The main difference is the addition of an ``haloswap`` routine to exchange the borders of the local grids between MPI ranks. This routine contains Point-to-Point communications of the Send/Recv kind.

.. code-block:: console

   !  Send upper boundaries and receive lower ones

   if (rank == 0) then
        call mpi_send(x(1,n), m, MPI_DOUBLE_PRECISION, rank+1, &
                     tag, comm, ierr)

   else if (rank == size-1) then

        call mpi_recv(x(1,0), m, MPI_DOUBLE_PRECISION, rank-1, &
                      tag, comm, status, ierr)

   else

        call mpi_sendrecv(x(1,n), m, MPI_DOUBLE_PRECISION, rank+1, tag, &
                          x(1,0), m, MPI_DOUBLE_PRECISION, rank-1, tag, &
                          comm, status, ierr)

   end if

   !  Send lower boundaries and receive upper ones

   if (rank == 0) then

        call mpi_recv(x(1,n+1), m, MPI_DOUBLE_PRECISION, rank+1, &
                      tag, comm, status, ierr)

   else if (rank == size-1) then

        call mpi_send(x(1,1), m, MPI_DOUBLE_PRECISION, rank-1, &
                     tag, comm, ierr)

   else

        call mpi_sendrecv(x(1,1)  , m, MPI_DOUBLE_PRECISION, rank-1, tag, &
                         x(1,n+1), m, MPI_DOUBLE_PRECISION, rank+1, tag, &
                         comm, status, ierr)
   end if


4.1 Enter the ``./mpi`` directory and generate a report tracing mpi routines. Which is the most time consuming MPI routine summarized over time (use the CLI)?

4.2 Enter the ``./mpi-nogpudirect`` directory and submit the jobfile to compile and run the instrumented application. The job will also generate summaries. Check the kind of data movement from the report and from the summaries (CUDA memcpy), as well as the time for MPI communications. Which is the ratio between kernel execution and memory+MPI communications?

4.4 Enter the ``./mpi-gpudirect`` directory and submit the jobfile to compile and run the instrumented application. Here, cuda-aware mpi communcations using GPUdirect on Leonardo are implemented. Check the time for MPI communications and the kind of data movements. Which additional communication do you spot? How does the time for D2H and H2D movements change? How does the ratio between kernels and data movement change?
