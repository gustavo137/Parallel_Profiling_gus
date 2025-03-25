---
title: Score-P

---

# Score-P

In this tutorial, we will implement the full workflow to profile an MPI application using Score-P/Scalasca.

The application is the MPI version of a Jacobi toy code. In this version of the code, the grid is distributed among MPI tasks along one direction. Jacobi step is followed by an MPI communication to exchange the boundaries at each iteration step. The communication is implemented as a Point-to-Point via Send/Recv operations. The scf loop contains also an Allreduce operation to compute the error; the latter is needed to check for convergence.

## Step 1 : Assess strong-scaling performance

Firstly, we need reference runs to asses code performance when increasing the number of MPI tasks. Compile the code and run with a varying number of MPI tasks.

**Strong scaling**: increase the number of MPI tasks without changing the overall workload (grid dimension).

From the slurm logfile, we can retrieve the runtime of the jacobi loop.

```
 Time for   1000 iterations size 1  was  0.7838     seconds
 Time for   1000 iterations size 2  was  0.4036     seconds
 Time for   1000 iterations size 5  was  0.1523     seconds
 Time for   1000 iterations size 10 was  0.5864E-01 seconds
 Time for   1000 iterations size 20 was  0.4204E-01 seconds
 Time for   1000 iterations size 40 was  0.4047E-01 seconds
```


![image](https://hackmd.io/_uploads/H1AQSw7Ykg.png)


## Step 2 : Instrument the application with Score-P

Score-P can be used to analyze time in user-defined routines and time in MPI. To instrument the application with Score-P, the code has to be compiled with

```
scorep --mpp=mpi <mpi-compiler> <src> -o <exe>
```

This generates an instrumented binary; to profile it, this can be run on a given number of mpi tasks with:

```
scalasca -analyze <mpi-launcher> -n N <exe> <args>
```

This will generate a folder with score-p files:

- scorep.cfg : contains the environment variables for score-P measurement ;
- scorep.log : contains the logfile
- profile.cubex : contains the profile

## Step 3 : Analyze the instrumentation footprint

Consider increasing the number of iterations to improve the performance statistics

Be aware that using profilers can increase the runtime due to overhead. Score-P provides ways to control the overhead and apply filters against it.

To check for Score-P requirements:

```
scorep-score scorep_exe_10_sum/profile.cubex
```

This shows how much memory is required by instrumentation ( set by SCOREP_TOTAL_MEMORY ) and which is the expected size of the trace ( aggregate size of event trace ).

```
Estimated aggregate size of event trace:                   99MB
Estimated requirements for largest trace buffer (max_buf): 5MB
Estimated memory requirements (SCOREP_TOTAL_MEMORY):       7MB
(hint: When tracing set SCOREP_TOTAL_MEMORY=7MB to avoid intermediate flushes
 or reduce requirements using USR regions filters.)
```

Setting the appropriate `SCOREP_TOTAL_MEMORY` is needed to avoid buffer flushes.

The size of the trace and the total memory required depends on the 
- duration of the run 
- amount of events instrumented 
- duration of the instrumented routines. 
 
Remind that Score-P instruments all user-defined APIs; if your code contains a number of short routines, this are more likely to be affected by instrumentation overhead.

## Step 3 : Characterize the application

This table shows the time spent in the different events. 

```
flt     type max_buf[B]    visits time[s] time[%] time/visit[us]  region
         ALL  5,227,177 2,838,917   31.83   100.0          11.21  ALL
         MPI  2,540,960   600,416   26.72    83.9          44.49  MPI
         USR  2,446,104 2,038,401    3.56    11.2           1.75  USR
         COM    240,096   200,080    1.55     4.9           7.74  COM
      SCOREP         41        20    0.01     0.0         256.88  SCOREP
```

How is your code bound?

:::spoiler
From the flat profile above, we can infer that the application is MPI bound (50% of the time is spent inside MPI routines).
:::
Now use the following command to prompt the full flat profile. 
```
scorep-score -r scorep_exe_10_sum/profile.cubex
```
This provides a flat profile of events instrumented. According to which metric are events listed? 

```
flt     type max_buf[B]    visits time[s] time[%] time/visit[us]  region
         ALL  5,227,177 2,838,917   31.83   100.0          11.21  ALL
         MPI  2,540,960   600,416   26.72    83.9          44.49  MPI
         USR  2,446,104 2,038,401    3.56    11.2           1.75  USR
         COM    240,096   200,080    1.55     4.9           7.74  COM
      SCOREP         41        20    0.01     0.0         256.88  SCOREP

         MPI         24        20   13.01    40.9      650561.78  MPI_Init
         MPI        118        38    4.94    15.5      129932.66  MPI_Ssend
         MPI         24        20    4.87    15.3      243409.90  MPI_Finalize
         MPI    660,066   200,020    2.47     7.8          12.35  MPI_Allreduce
         USR    240,000   200,000    1.83     5.8           9.17  jacobi.jacobistep_
         USR    240,000   200,000    1.54     4.8           7.69  jacobi.deltasq_
         MPI  1,880,188   360,036    1.30     4.1           3.61  MPI_Sendrecv
         COM         24        20    0.95     3.0       47285.95  MAIN__
         COM         24        20    0.55     1.7       27318.24  cfdio.writedatafiles_
         USR    491,520   409,600    0.11     0.3           0.27  cfdio.hue2rgb_
         MPI    592,301    20,040    0.10     0.3           4.94  MPI_Recv
         USR  1,474,560 1,228,800    0.08     0.2           0.06  cfdio.colfunc_
         COM    240,024   200,020    0.06     0.2           0.28  boundary.haloswap_
         MPI    590,059    20,002    0.01     0.0           0.63  MPI_Send
         MPI        264        80    0.01     0.0         136.70  MPI_Bcast
         MPI        132        40    0.01     0.0         159.19  MPI_Barrier
      SCOREP         41        20    0.01     0.0         256.88  exe
         USR         24         1    0.00     0.0        1319.79  cfdio.writeplotfile_
         MPI         72        60    0.00     0.0           1.01  MPI_Comm_rank
         COM         24        20    0.00     0.0           2.25  boundary.boundarypsi_
         MPI         72        60    0.00     0.0           0.25  MPI_Comm_size
```
:::spoiler
Events are listed according to the maximum buffer size.
:::

Which are the most time consuming routines?

To order the flat profile according time, use
```
scorep-score -r -s totaltime scorep_exe_10_sum/profile.cubex
```
it will prompt a reordered flat profile:
```
flt     type max_buf[B]    visits time[s] time[%] time/visit[us]  region
         ALL  5,227,177 2,838,917   31.83   100.0          11.21  ALL
         MPI  2,540,960   600,416   26.72    83.9          44.49  MPI
         USR  2,446,104 2,038,401    3.56    11.2           1.75  USR
         COM    240,096   200,080    1.55     4.9           7.74  COM
      SCOREP         41        20    0.01     0.0         256.88  SCOREP

MPI  66,000,066 10,000,010   91.78    36.2           9.18  MPI_Allreduce
 USR  24,000,000 10,000,000   51.68    20.4           5.17  jacobi.jacobistep_
 USR  24,000,000 10,000,000   37.19    14.7           3.72  jacobi.deltasq_
 COM          24         10   31.17    12.3     3116737.05  MAIN__
 MPI 188,000,188 16,000,016   25.89    10.2           1.62  MPI_Sendrecv
 MPI          24         10    5.59     2.2      558631.04  MPI_Init
 MPI  59,001,121  2,000,020    4.79     1.9           2.40  MPI_Recv
 COM  24,000,024 10,000,010    2.80     1.1           0.28  boundary.haloswap_
 MPI  59,000,059  2,000,002    1.12     0.4           0.56  MPI_Send
 MPI         118         18    0.78     0.3       43159.42  MPI_Ssend
 MPI          24         10    0.74     0.3       73553.30  MPI_Finalize
 COM          24         10    0.16     0.1       16140.75  cfdio.writedatafiles_
 USR     245,760    102,400    0.03     0.0           0.27  cfdio.hue2rgb_
 USR     737,280    307,200    0.02     0.0           0.06  cfdio.colfunc_
 USR          24          1    0.01     0.0       13727.91  cfdio.writeplotfile_
 MPI         264         40    0.00     0.0         110.38  MPI_Bcast
 MPI         132         20    0.00     0.0         135.10  MPI_Barrier
SCOREP          41         10    0.00     0.0         215.74  exe
 MPI          72         30    0.00     0.0           1.04  MPI_Comm_rank
 COM          24         10    0.00     0.0           2.32  boundary.boundarypsi_
 MPI          72         30    0.00     0.0           0.27  MPI_Comm_size
```
Do you think that the percentage of time spent in MPI is reliable?

:::spoiler
Warning! 80% of the time is in MPI but a significant amount is in initialization/finalization routines, which are non negligible here due to the low number of iteration steps. Time spent in relevant communication routines is about 46%.
:::

## Step 3 : Filter the application

The previous summary shows that the events with the largest buffer requirement are MPI_Sendrecv and another routine used for IO operations at the end of the program. Max buffer indicates how much the event weights in the final size of the trace.

Regarding the second event, we observe that the time/visit is siginifcantly low, thus being more prone to instrumentation overhead. To reduce the size of the event trace and avoid this overhead, we can tell Score-p not to instrument specific routines.

```
Estimated aggregate size of event trace:                   99MB
Estimated requirements for largest trace buffer (max_buf): 5MB
Estimated memory requirements (SCOREP_TOTAL_MEMORY):       7MB
(hint: When tracing set SCOREP_TOTAL_MEMORY=7MB to avoid intermediate flushes
 or reduce requirements using USR regions filters.)

flt     type max_buf[B]    visits time[s] time[%] time/visit[us]  region
         ALL  5,227,177 2,838,917   31.83   100.0          11.21  ALL
         MPI  2,540,960   600,416   26.72    83.9          44.49  MPI
         USR  2,446,104 2,038,401    3.56    11.2           1.75  USR
         COM    240,096   200,080    1.55     4.9           7.74  COM
      SCOREP         41        20    0.01     0.0         256.88  SCOREP

         MPI  1,880,188   360,036    1.30     4.1           3.61  MPI_Sendrecv
         USR  1,474,560 1,228,800    0.08     0.2           0.06  cfdio.colfunc_
         MPI    660,066   200,020    2.47     7.8          12.35  MPI_Allreduce
         MPI    592,301    20,040    0.10     0.3           4.94  MPI_Recv
         MPI    590,059    20,002    0.01     0.0           0.63  MPI_Send
         USR    491,520   409,600    0.11     0.3           0.27  cfdio.hue2rgb_
         COM    240,024   200,020    0.06     0.2           0.28  boundary.haloswap_
         USR    240,000   200,000    1.83     5.8           9.17  jacobi.jacobistep_
         USR    240,000   200,000    1.54     4.8           7.69  jacobi.deltasq_
         MPI        264        80    0.01     0.0         136.70  MPI_Bcast
         MPI        132        40    0.01     0.0         159.19  MPI_Barrier
         MPI        118        38    4.94    15.5      129932.66  MPI_Ssend
         MPI         72        60    0.00     0.0           1.01  MPI_Comm_rank
         MPI         72        60    0.00     0.0           0.25  MPI_Comm_size
      SCOREP         41        20    0.01     0.0         256.88  exe
```

We need to build a filter.file where we specify the name of the routine we want to exclude. 

```
SCOREP_REGION_NAMES_BEGIN
  EXCLUDE
          cfdio.colfunc_
SCOREP_REGION_NAMES_END
```
To estimate the gain after filtering:
```
 scorep-score -f filter.file -r scorep/profile.cubex | more
```
Now the report is 20MB smaller. This might seem a small achievement, but in traces of large codes with many routine this tool can make the difference to generate affordable traces :)

```
Estimated aggregate size of event trace:                   71MB
Estimated requirements for largest trace buffer (max_buf): 3665kB
Estimated memory requirements (SCOREP_TOTAL_MEMORY):       6MB
(hint: When tracing set SCOREP_TOTAL_MEMORY=6MB to avoid intermediate flushes
 or reduce requirements using USR regions filters.)

flt     type max_buf[B]    visits time[s] time[%] time/visit[us]  region
 -       ALL  5,227,177 2,838,917   31.83   100.0          11.21  ALL
 -       MPI  2,540,960   600,416   26.72    83.9          44.49  MPI
 -       USR  2,446,104 2,038,401    3.56    11.2           1.75  USR
 -       COM    240,096   200,080    1.55     4.9           7.74  COM
 -    SCOREP         41        20    0.01     0.0         256.88  SCOREP

 *       ALL  3,752,617 1,610,117   31.75    99.8          19.72  ALL-FLT
 -       MPI  2,540,960   600,416   26.72    83.9          44.49  MPI-FLT
 +       FLT  1,474,560 1,228,800    0.08     0.2           0.06  FLT
 *       USR    971,544   809,601    3.48    10.9           4.30  USR-FLT
 *       COM    240,096   200,080    1.55     4.9           7.74  COM-FLT
 -    SCOREP         41        20    0.01     0.0         256.88  SCOREP-FLT

 -       MPI  1,880,188   360,036    1.30     4.1           3.61  MPI_Sendrecv
 +       USR  1,474,560 1,228,800    0.08     0.2           0.06  cfdio.colfunc_
 -       MPI    660,066   200,020    2.47     7.8          12.35  MPI_Allreduce
 -       MPI    592,301    20,040    0.10     0.3           4.94  MPI_Recv
 -       MPI    590,059    20,002    0.01     0.0           0.63  MPI_Send
 -       USR    491,520   409,600    0.11     0.3           0.27  cfdio.hue2rgb_
 -       COM    240,024   200,020    0.06     0.2           0.28  boundary.haloswap_
 -       USR    240,000   200,000    1.83     5.8           9.17  jacobi.jacobistep_
 -       USR    240,000   200,000    1.54     4.8           7.69  jacobi.deltasq_
 -       MPI        264        80    0.01     0.0         136.70  MPI_Bcast
 -       MPI        132        40    0.01     0.0         159.19  MPI_Barrier
```



## Step 4 : Run with filter and postprocess 

Score-P provides a number of environment variables to customize the measurement. In particular we mention:

```
export SCOREP_ENABLE_TRACING=OFF
export SCOREP_ENABLE_PROFILING=ON
export SCOREP_EXPERIMENT_DIRECTORY=scorep_run
export SCOREP_METRIC_PAPI=PAPI_TOT_INS,PAPI_TOT_CYC
```
If Score-P is configured with PAPI, we can use envrironment variables to read hardware counters with the library.

Now you can rerun your measurement with the above environment variables and 

```
scalasca -analyze -f filter.file
```
to actually implement a measurement with the filter applied.

Score-P instrumentation generates main metrics (e.g. time, number of visits). To produce derived metrics, the profile.cubex has to be postprocessed with scalasca -examine.

```
 scalasca -examine scorep_exe_10_sum
```

This generates a summary.cubex to be opened with CUBE gui.

![image](https://hackmd.io/_uploads/BycvITRuJx.png)


![image](https://hackmd.io/_uploads/Sk_rIa0uyl.png)

![image](https://hackmd.io/_uploads/BJ6CUaAd1g.png)

## Step 5 : User instrumentation

To focus the performance metrics on a given region of the source code, we can annotate ranges with SCOREP_USER_REGION. To use these, firstly add

```
#include "scorep/SCOREP_User.inc"
SCOREP_USER_REGION_DEFINE( myhandle )
```

and then annotate the source code region with

```
 SCOREP_USER_REGION_BEGIN( myhandle, "block", SCOREP_USER_REGION_TYPE_COMMON )
 <code here>
 SCOREP_USER_REGION_END( myhandle )
```

![image](https://hackmd.io/_uploads/SyX6Y6AOke.png)

![image](https://hackmd.io/_uploads/BkAe9TA_yl.png)

Inside the loop MPI rank 0 is spending some more time to print; increase the printfreq

![image](https://hackmd.io/_uploads/Skga5pRdkg.png)

Load balance increases significantly and we gain about 30% of time! To target communications tracing is needed.

## Step 5 : Trace the application

```
scalasca -analyze -t <mpi-launcher> -n N <exe> <args>
```