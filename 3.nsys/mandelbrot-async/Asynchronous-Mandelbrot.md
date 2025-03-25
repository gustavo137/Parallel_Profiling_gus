Asynchronous Mandelbrot
=======================

This code is taken from the repository of [OpenACC best practice guide](https://github.com/OpenACC/openacc-best-practices-guide/tree/main/examples/mandelbrot) from NVIDIA and reproduces a common operation in image processing. The image is loaded on an array, each element of this array corresponds to a pixel. Each pixel is processed from within a loop by a mandelbrot function. 

The code is composed by two files:
- `main.*` : containing the main loop of the program;
- `mandelbrot.*` : containing the definition of the mandelbrot function.

Offload
-------

1. As a first step, we are going to offload the double loop with OpenACC. :exclamation: Be careful, the double loop contains a function, which has to be executed by GPU threads on the device. Manage data movements explicitely and use if feasible clauses for loop optimization.
2. Modify the Makefile in order to accelerate on GPUs with `-Minfo=accel`, use `-gpu=pinned` in order to pin memory.
3. Modify the jobscript to compile, run the simulation and trace openacc runtime and cuda APIs.

Once the job is done, reply to the following questions:

- how long did the loop calculation take?
- Open the trace with `nsys-ui` and check from the profilers. Focus on the ported part. How much is time spent in data movements versus kernel computation. Can you guess a possible strategy to reduce the time to solution?

Blocking
--------

Let's now divide the image array into chuncks along the leading direction. To this:

1. Define as a parameter `NUM_BLOCKS`, and assign it a value. This number is the number of chuncks for the image array. 
2. Declare a block iteration and a block size, define the latter according to the number of blocks.
3. Modify the double loop and add an iteration over array chuncks.
4. Offload the blocked loop by taking care of data movements; instead of copying out the final data at the end of the loops, use the `update` clause to update the chuncks of image as soon as these are computed.
5. Resubmit the job.

Asnwers to the following questions:

- Do you observe any relevant difference in the time to solution?
- Open the trace with `nsys-ui` and look at the trace. Can you guess how we could reduce the time to solution?

Asynchronous blocking
---------------------

In order to reduce the time to solution, we need to exploit streams to overlap data movements with computation. To this the kernels and the data download are queued on different streams, one per block or eventually less:

1. add the `async()` clause where needed.
2. add the `wait` clause where needed.
3. Resubmit the job.

Answers the following questions:

- Do you observe a speed up in time to solution?
- Open the report and check the timeline. How many streams are now used? Is there overlap among data and computation?
- Repeat the same measurement with different block sizes and a different number of streams, discuss the bahviour.

Solutions
---------
To offload the mandelbrot routine called from the parallel loop, we need to label the routine as a `device routine seq`, to be executed sequentially by each GPU thread.

In the non blocking version of the code, the runtime spends a significant amount of time in downloading data, once the computing part is efficiently offloaded to GPU.

![image](https://hackmd.io/_uploads/SkKgYtIl1g.png)

We now try hiding the cost of D2H data movemenets by overlapping it with some computation. To this, we divide the image array into a number of blocks, and add a loop over the blocks. In this view, the final download is replaced by a partial update on the host once the block is computed.

![image](https://hackmd.io/_uploads/HJkKHYUgye.png)

If the blocks are sent to the same queue, as done by default by OpenACC, there is no overlap and thus no performance gain. To enable concurrency on different streams, the `async` clause has to be added to the `parallel` and the `update` clause. We can set a number of streams, chuncks of array are processed to each stream in a round robin fashion, and the result is the one below. We can play with the number of blocks and streams available to identify the best number of block and number of streams. 

![image](https://hackmd.io/_uploads/SkD98K8xyx.png)


Can you do better than the best below?

|Version | Time (s) |
|--------|----------|
| non-blocking | 3.6880970E-02 |
| blocking     | 3.4833193E-02 |
| async, 2 streams 8 blocks | 1.7216921E-02 |
