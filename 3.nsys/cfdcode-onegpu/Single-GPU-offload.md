
CFD single GPU
=============

This code is taken from the repository of [David Henty](https://github.com/davidhenty/cfd/tree/master) and implements a regular 2D CFD simulation of an incompressible fluid flowing in a cavity using the Navier-Stokes equation.

The code is composed by 4 files

- `boundary.f90` : buondary initialization
- `cfdio.f90` : defines routines for IO operations at the end of the program
- `jacobi.f90` : contains routines to implement the jacobi step and the error calculation
- `cfd.f90` : main of the program, implements the main loop for cfd code 
- `nvtx.f90` : wrappers to nvtxExtTools library

Reference run
---------------

Generate a reference run in `serial/` folder:

1. Modify the makefile in order to compile with `nvhpc/24.3` and `-fast` and `-g` options.
2. Add to the jobscript the instructions to (i) load the module, (ii) compile the application, (iii) run a simulation with inputs 200 50, (iii) trace the application with nsys profile `--backtrace=dwarf`
3. Submit the jobscript.

### Questions

- Check from the logfile the time to execute one loop iteration.
- Open the report with `nsys-ui`, identify the 2 most time consuming events from the summary panel; use the bottom-up view to identify the routine calling the first event. Is this routine more likely triggered from computation or some IO?

Instrumentation
---------------

Instrument the application in `nvtx/` folder:

1. Add nvtx ranges in the source code; wrap the initialization, main loop, finalization and main routines called within the loop.
2. Add `use nvtx` in the header of the program
3. Modify the `Makefile` to link against the nvtxExtTools library.
4. Modify the jobscript to instrument also nvtx ranges.

### Questions

- Which is the most time consuming phase of the program, and the % of runtime spent in this phase?
- Is this part suitable for offload?

Filtering
---------
This code contains a I/O phase at the end of the program which is not interesting for the offload. To avoid collecting unnecessary data, we focus our tracing on the main loop, to be the target of GPU offload.

1. Modifiy the `nsys profile` command in the jobscript to trace only the main loop region; submit the jobscript.
2. Open the report; check the overall duration of the trace and the Flat summary from the bottom panel.

### Questions

- Which is the most time consuming event reported now in the Flat summary?

Accelerate
----------

At this stage, we start the offload assisted by NSight Systems traces. 

1. Add `parallel loop` directives to the loop in `jacobi` routine and in the main program, without optimization clauses. 
2. Modify the `Makefile` in order to compile OpenACC directives targeting GPUs. Add also `-Minfo=accel` for compiler feedbacks.
3. Modify the jobscript in order to trace also OpenACC runtime and submit.

### Questions

- From the logfile, how much time does an iteration take?
- Open the report with `nsys-ui`. How much time does the computation take with respect to data movement?
- Can you identify the OpenACC directives corresponding to compute region and data region? Are the latter explicit or implicit?
- Regarding the compute region, how many gangs worker and which vector length are used? How does this map to the block and grid sizes?
- Can you identify the amount of time spent in kernel launches for the loops?
- Open the logfile and check the offload report from the compiler. Which implicit operation did the compiler do? 

Optimize
--------

Now the real challenge: improve the offload bt taking care of data movements and loop optimizations.

Solution
-------

### Reference run

The time for each iteration is **0.1352 seconds**

|Symbol Name|	Self, %|	Stack, %|	
|------|-------|-------|
| \_f90io\_encode\_fmt	| 19.39	|20.21	|
| jacobi\_jacobistep\_acc\_|	8.86	|8.95|

The first event is called from `writedatafiles` routine.

![image](https://hackmd.io/_uploads/rJABN7rgye.png)

### Instrumentation

The code implements an IO phase at the end of the simulation, the one labelled by *write_out* in the timeline view below. This phase is about 75% of the program runtime 

![image](https://hackmd.io/_uploads/SkVX9vwxke.png)


### Filtering

By filtering out the IO phase, traces and summaries show only events related to the main loop. The jacobi step now takes about 35% of the traced time. This should be the first target of our offload.

![image](https://hackmd.io/_uploads/H1fWg4Hl1x.png)

### Accelerate

The time for each iteration is **0.1608 seconds**

As a first attemp to accelerate the code, we offload with parallel constructs the loops inside the jacobi routine and a loop inside the main program. However, without taking care explicitely of data movements, the runtime copies data when opening and closing each data region. Thus, most of the GPU time is spent by moving data (0.8% vs 99.2%) as visible from the image below.

![image](https://hackmd.io/_uploads/B16REVrx1e.png)

Nsight Systems highlights with different colors the data uploaded (green) and downloaded (red). By looking at the OpenACC trace, we can also identify the source code line generating this data movement ( line 144 from `cfd.f90` and 14 in `jacobi.f90` ).

We can identify the data moved, by putting the cursor on the "enqueue upload" range.

![image](https://hackmd.io/_uploads/rkDx2vPgJl.png)

We can also observe a gap in GPU activity (highlighted below) when the runtime is in `deltasq` routine from jacobi module (sampling point).

![image](https://hackmd.io/_uploads/H1MhowDlJl.png)

The time spent in kernel launches is instead relatively small compared to the time spent by the GPU in executing the kernel on the GPU

![image](https://hackmd.io/_uploads/BkV7HESeyx.png)

Also from the OpenACC summary below we can infer that most of the OpenACC time is spent in moving data in and out the GPU.

![image](https://hackmd.io/_uploads/rym3rErlke.png)

Optimize
--------

The time for each iteration is **0.009194 seconds**

Data do not need to be moved in and out the GPU at each `parallel` directive. By using explicit data directives, we can copyin/create the arrays before the while loop, and copyout/delete the arrays after the while loop. To preserve data locality within the loop, we need to offload also the reduction in `deltasq` routine.


![image](https://hackmd.io/_uploads/HyBUkODl1g.png)


The picture above shows now 75.5% of the time spent in computing vs 24.5% spent in data management. Data are moved only at the beginning and at the end of the while loop.

Be careful when you instrument short programs, or filter short ranges/routines : if you look at the whole trace (picture below) you can see that the time spent in *main\_loop* is affected by OpenACC profiling initialization overhead. Nsight systems provide a "Profile overhead" row and from here the impact of profiling overhead can be inferred.

![image](https://hackmd.io/_uploads/By-XlOPxJx.png)





