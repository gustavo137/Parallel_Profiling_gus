#include <mpi.h>
#include <iostream>

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);

    int world_rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

    int broadcast_data;
    if (world_rank == 0) {
        // Root process (rank 0) sets the data
        broadcast_data = 100;
        std::cout << "Process 0 broadcasting data: " << broadcast_data << std::endl;
    }

    // Broadcast the data from process 0 to all other processes
    MPI_Bcast(&broadcast_data, 1, MPI_INT, 0, MPI_COMM_WORLD);

    // All processes, including root, now have the data
    std::cout << "Process " << world_rank << " has data: " << broadcast_data << std::endl;

    MPI_Finalize();
    return 0;
}
