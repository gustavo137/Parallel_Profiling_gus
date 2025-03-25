#include <mpi.h>
#include <stdio.h>

int main(int argc, char** argv) {
    // Initialize the MPI environment
    MPI_Init(&argc, &argv);

    // Get the number of processes
    int world_size; //also commomly called "nps" or "npes"
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    // Get the rank of the process
    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    printf("Hello world from  rank %d out of %d processors\n", rank, world_size);

    // Finalize the MPI environment.
    MPI_Finalize();
}

