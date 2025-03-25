#include <iostream>
#include <mpi.h>
#include <stdio.h>

// Template Function Declaration
template <typename T>
void ring_communication(T &world_value, int world_rank, int world_size);

// Speialization template for int
template <>
void ring_communication<int>(int &world_value, int world_rank, int world_size) {
    int prev_rank = (world_rank - 1 + world_size) % world_size;
    int next_rank = (world_rank + 1) % world_size;

    int received_value;
    MPI_Sendrecv(&world_value, 1, MPI_INT, next_rank, 0,
                 &received_value, 1, MPI_INT, prev_rank, 0,
                 MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    world_value = received_value;
}

// Specialization template for double
template <>
void ring_communication<double>(double &world_value, int world_rank, int world_size) {
    int prev_rank = (world_rank - 1 + world_size) % world_size;
    int next_rank = (world_rank + 1) % world_size;

    double received_value;
    MPI_Sendrecv(&world_value, 1, MPI_DOUBLE, next_rank, 0,
                 &received_value, 1, MPI_DOUBLE, prev_rank, 0,
                 MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    world_value = received_value;
}

// Default case uses MPI_BYTE
template <typename T>
void ring_communication(T &world_value, int world_rank, int world_size) {
    int prev_rank = (world_rank - 1 + world_size) % world_size;
    int next_rank = (world_rank + 1) % world_size;

    T received_value;
    MPI_Sendrecv(&world_value, sizeof(T), MPI_BYTE, next_rank, 0,
                 &received_value, sizeof(T), MPI_BYTE, prev_rank, 0,
                 MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    world_value = received_value;
}

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);

    int world_rank, world_size;
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);
/*
    // Example with int
    int world_value_int = world_rank;
    ring_communication(world_value_int, world_rank, world_size);
    printf("Process %d (int) has the value: %d\n", world_rank, world_value_int);

    // Example with double
    double world_value_double = (double)world_rank + 0.5;
    ring_communication(world_value_double, world_rank, world_size);
    printf("Process %d (double) has the value: %f\n", world_rank, world_value_double);
*/
    //General  example
    //int world_value_gen = world_rank;
    double world_value_gen = (double)world_rank+0.5;
    ring_communication(world_value_gen, world_rank, world_size);
    //printf("Process %d (MPI_BYTE) has the value: %d\n", world_rank, world_value_gen);
    printf("Process %d (MPI_BYTE) has the value: %f\n", world_rank, world_value_gen);


    MPI_Finalize();
    return 0;
}

