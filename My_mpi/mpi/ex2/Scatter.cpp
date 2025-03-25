#include <mpi.h>
#include <iostream>

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);

    int world_rank, world_size;
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    const int data_per_process = 2;  // Number of elements each process will receive
    int *send_data = nullptr;

    if (world_rank == 0) {
        // Root process initializes an array to send to all processes
        send_data = new int[world_size * data_per_process];
        for (int i = 0; i < world_size * data_per_process; i++) {
            send_data[i] = i;  // Fill with some data (0, 1, 2, ...)
        }
        std::cout << "Root process sending data: ";
        for (int i = 0; i < world_size * data_per_process; i++) {
            std::cout << send_data[i] << " ";
        }
        std::cout << std::endl;
    }

    // Each process will receive 2 elements
    int recv_data[data_per_process];

    // Scatter the data: each process gets a chunk of 2 elements from send_data
    MPI_Scatter(send_data, data_per_process, MPI_INT, recv_data, data_per_process, MPI_INT, 0, MPI_COMM_WORLD);

    // Each process prints the data it received
    std::cout << "Process " << world_rank << " received data: ";
    for (int i = 0; i < data_per_process; i++) {
        std::cout << recv_data[i] << " ";
    }
    std::cout << std::endl;

    // Clean up dynamically allocated memory
    if (world_rank == 0) {
        delete[] send_data;
    }

    MPI_Finalize();
    return 0;
}

