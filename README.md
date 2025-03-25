# Profiling-tutorial-mhpc

## Interactive session

Compute nodes can be reached via interactive sessions:

```
ssh -N 1 --ntasks-per-node=1 --cpus-per-task=6 --gres:gpu=1 -p <queue> --pty /bin/bash
```

More information can be found in the [User Guide](https://wiki.u-gov.it/confluence/display/SCAIUS/Clusters+Specifics)

This folder contains 3 exercises

- **gprof** : Analyze a serial code with Gprof
- **score-P** : Use Score-P and Scalasca to profile an MPI job and detect bottlenecks
- **nsys**: Use NSight Systems to detect (i) detect bottlenecks in single-gpu code, (ii) implement multistream, (iii) analyze MPI awareness in multi-GPU simulation.
