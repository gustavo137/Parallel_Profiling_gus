#include <mpi.h>
#include <iostream>

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);

    int world_rank, world_size;
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    // Each process has an array of different size to send
    int send_count = world_rank + 1;  // Process 0 sends 1 element, Process 1 sends 2, etc.
    int *send_data = new int[send_count];
    for (int i = 0; i < send_count; i++) {
        send_data[i] = world_rank * 1 + i;  // Fill the array with unique data
    }

    // The root process needs to know how much data to expect from each process
    int *recv_counts = nullptr;
    int *displacements = nullptr;
    int total_recv_count = 0;
    int *recv_data = nullptr;

    if (world_rank == 0) {
        recv_counts = new int[world_size];
        displacements = new int[world_size];
        for (int i = 0; i < world_size; i++) {
            recv_counts[i] = i + 1;  // Each process sends i+1 elements
            displacements[i] = total_recv_count;  // Calculate where to put each chunk
            total_recv_count += recv_counts[i];   // Total number of elements to receive
        }
        recv_data = new int[total_recv_count];  // Buffer to gather all data
    }

    // Use MPI_Gatherv to gather data from all processes at root (rank 0)
    MPI_Gatherv(send_data, send_count, MPI_INT, recv_data, recv_counts, displacements, MPI_INT, 0, MPI_COMM_WORLD);

    // Root process prints the received data
    if (world_rank == 0) {
        std::cout << "Root process gathered data: ";
        for (int i = 0; i < total_recv_count; i++) {
            std::cout << recv_data[i] << " ";
        }
        std::cout << std::endl;
    }

    // Clean up dynamically allocated memory
    delete[] send_data;
    if (world_rank == 0) {
        delete[] recv_counts;
        delete[] displacements;
        delete[] recv_data;
    }

    MPI_Finalize();
    return 0;
}

