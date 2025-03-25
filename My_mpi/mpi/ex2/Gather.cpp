#include <mpi.h>
#include <iostream>
#include <vector>

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);

    int world_rank, world_size;
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    // Each process sends this data
    int send_data = world_rank;

    // The root process will gather the data into this receive buffer
    std::vector<int> recv_data;
    if (world_rank == 0) {
        recv_data.resize(world_size);  // Only the root process needs to allocate the recv buffer
    }

    // Gather data from all processes to root process (process 0)
    MPI_Gather(&send_data, 1, MPI_INT, recv_data.data(), 1, MPI_INT, 0, MPI_COMM_WORLD);

    // Root process prints the gathered data
    if (world_rank == 0) {
        std::cout << "Process 0 gathered data: ";
        for (int i = 0; i < world_size; ++i) {
            std::cout << recv_data[i] << " ";
        }
        std::cout << std::endl;
    }

    MPI_Finalize();
    return 0;
}
