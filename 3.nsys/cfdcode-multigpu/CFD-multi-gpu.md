CFD multi-gpu with MPI
=============

The non-gpu version of this code is taken from the repository of [David Henty](https://github.com/davidhenty/cfd/tree/master).

This is the MPI-distributed version of the CFD previously offloded to a single GPU. In this case, each MPI rank will be binded to a GPU for the offload. The aim is to implement efficient GPU to GPU communications.

The code is composed by four files 

- `cfd.f90` contains the main loop of the program
- `jacobi.f90` contains the jacobi step the reduction for error calculation
- `boundary.f90` contains initialization routine and the haloswap
- `cfdio.f90` contains routines for IO.

Non-GPU run
-----------

Instrument the `haloswap` and the main loop with nvtx ranges. Run the non GPU case e trace the application with `nsys`; how much time is spent in communications (haloswap) within the loop?

MPI GPU-binding
---------------

Implement MPI-GPU binding in a round robin fashion, by using the following routines

- `acc_get_device_type`
- `acc_get_num_devices`
- `acc_set_device_num`
- `acc_get_device_num`

Check MPI-GPU binding the following message

`write(*,*) "MPI ", rank, " is using GPU: ", acc_get_device_num(acc_get_device_type())`

Identify which routine contains local compuation to offload, and which routine require communications among them. 

MPI non aware implementation
----------------------------

As a first step, add data clauses to update buffers between CPU and GPU before and after the communications; move only the portion of the data needed (you can do array slicing). Modify the jobscript in order to trace also openacc and cuda, submit. Open the report with `nsys-ui`.

- How much time does communication take?
- Which is the ratio among MPI communication and computation inside the main loop?
- Can you identify a potential performance improvement?

MPI aware implementation
------------------------

Use the `host_data` directive to pass the buffer to the MPI APIs where needed. Repeat the run and check for the different time spent in MPI communications, the amount of data moved and the kind of memory operations.

Solution
--------

From the NVTX range summary we can infer the amount of time spent in MPI communications in the cpu case

 ** NVTX Range Summary (nvtx_sum):

 | Time (%) |  Total Time (ns)|  Instances  |    Avg (ns)  |         Med (ns)   |       Min (ns)|         Max (ns)  |    StdDev (ns) |   Style |    Range|
| -------- | ----------------- | --------- | ----------------- | ----------------- | --------------- | --------------- | ------------ | ------- | ---------|
|     97.7 | 1,753,684,405,470 |         4 | 438,421,101,367.5 | 438,421,102,361.5 | 438,421,088,229 | 438,421,112,518  |    10,131.0 | PushPop  |main_loop|
|      2.3  |   40,466,134,443   |   2,000 |      20,233,067.2   |    10,784,101.0 |         451,104    |   43,813,262 | 19,334,728.8 | PushPop | haloswap|

and in the non aware case.

 ** NVTX Range Summary (nvtx_sum):

|Time (%)|  Total Time (ns)|  Instances|     Avg (ns)|         Med (ns)|        Min (ns)|       Max (ns)|     StdDev (ns)|   Style|     Range|
| --------|---------------|---------|---------------|---------------|-------------|-------------|-----------|-------|---------|
|     89.1 |  17,895,932,730 |         4  |4,473,983,182.5 | 4,474,018,142.5 | 4,473,875,910|  4,474,020,535  |   71,542.9  |PushPop|  main_loop|
|     10.9  |  2,189,625,075 |     2,000     |1,094,812.5|      1,089,337.5|        949,352|      2,607,153  |   73,311.2 | PushPop  | haloswap |

Offloading operations on the GPU has exposed the time spent in MPI communications.

![image](https://hackmd.io/_uploads/BkvfA7Yekg.png)

The time line shows that data movements are done before and after the MPI communications to synchronize CPU and GPU. To improve the efficiency we implement awareness by passing to MPI APIs the device variables.

![image](https://hackmd.io/_uploads/H1r4CQFxJl.png)

MPI APIs implement P2P data movements visible from the profile

![image](https://hackmd.io/_uploads/BJoIVEYlyl.png)

