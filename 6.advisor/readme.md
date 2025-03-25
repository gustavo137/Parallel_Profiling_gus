## Get informations about your hardware

To get some informations about your processor

`lscpu`

Obtaining information on the devices on the PCI bus can be helpful, particularly for identifying the number and type of the graphics processor. The following command can be useful

`lspci`



## Measure the bandwidth with STREAMS

In order to measure the effective bandwidth we can

```
git clone git@github.com:jeffhammond/STREAM.git
cd  STREAM
```

Then, take a compute node and edit the Makefile, using for example using GNU compiler

```
CC = gcc
CFLAGS = -O3 -march=native -fstrict-aliasing -ftree-vectorize -fopenmp -DSTREAM_ARRAY_SIZE=80000000 -DNTIMES=20

FC = gfortran
FFLAGS = -O2 -fopenmp

all: stream_f.exe stream_c.exe

stream_f.exe: stream.f mysecond.o
        $(CC) $(CFLAGS) -c mysecond.c
        $(FC) $(FFLAGS) -c stream.f
        $(FC) $(FFLAGS) stream.o mysecond.o -o stream_f.exe

stream_c.exe: stream.c
        $(CC) $(CFLAGS) stream.c -o stream_c.exe

clean:
        rm -f stream_f.exe stream_c.exe *.o
```

Check that gcc module is loaded (it should be by default). In case you want to use Intel compiler, load the intel-oneapi-compilers module. Then,

``` 
make ./stream_c.exe
./stream_c.exe 
```

The output shall look like

```
Function    Best Rate MB/s  Avg time     Min time     Max time
Copy:          174552.4     0.007365     0.007333     0.007406
Scale:         146250.5     0.008777     0.008752     0.008812
Add:           159919.4     0.012024     0.012006     0.012042
Triad:         160521.9     0.011980     0.011961     0.012003
```


## Roofline model with Intel Advisor

In `roofline_intel_advisor/src` you can find some simple examples of matrix matrix multiplication in C and C++.
Feel free to modify / optimize the code as you like, it is just a playground. You can also modify the problem size. Correct errors if you find.


Start with just the serial matrix matrix multiplication and consider the roofline model using Intel® Advisor.
Pay attention to the compiler flags you use when compiling (such as the level of optimization or the vectorization like AVX or AVX512, try also to use single precision and double precision).
Recall to load the Intel Advisor module `module load intel-oneapi-advisor/2023.2.0-6eiojq7`. In general, you can follow as an example the job script `roofline_intel_advisor/launch.sh`
In general, the command look like something like the following, where `--with-stack` and `--memory-level=L1_L2_L3_DRAM` add additional informations in the report.

```
advisor --collect=survey       --project-dir=advisor_proj_cpp  -- ./main_cpp.x
advisor --collect=tripcounts   --project-dir=advisor_proj_cpp  --flop  --stacks --enable-cache-simulation  --  ./main_cpp.x
advisor --report=roofline      --project-dir=advisor_proj_cpp  #--with-stack  ##--memory-level=L1_L2_L3_DRAM
advisor --report=survey        --project-dir=advisor_proj_cpp 
```

A message will tell you where report is saved, something like in `PATHtoTheFOLDER/profiling-tutorial-mhpc/6.advisor/roofline/bin/advisor_proj_cpp/rank.0/hs000/advisor-roofline.html`.

Then on you laptop you can `scp` the `advisor-roofline.html` and open in your browser.

`scp YourAccount@login02-ext.g100.cineca.it:PATHtoTheFOLDER/profiling-tutorial-mhpc/6.advisor/roofline/bin/advisor_proj_simp/rank.0/hs000/advisor-roofline.html . `

Add multi-threading and vectorization and check the differences.

Finally, consider a more cache-friendly matrix matrix multiplication.

Look at the documentation for all the possible options!


