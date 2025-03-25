#include <mpi.h>
#include <iostream>
#include <vector>

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);

    int world_rank, world_size;
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    const int array_size = 10;  // Total size of the vector
    const int stride = 3;
    const int num_blocks = array_size / stride;  // 3 complete blocks
    const int remainder = array_size % stride;   // Remaining elements

    std::vector<int> data(array_size);

    // Initialize the vector differently for each process
    if (world_rank == 0) {
        for (int i = 0; i < array_size; ++i) {
            data[i] = i + 1; // Process 0 has [1, 2, 3, ..., 10]
        }
    } else {
        for (int i = 0; i < array_size; ++i) {
            data[i] = -(i + 1); // Process 1 has [-1, -2, -3, ..., -10]
        }
    }

    // Create a new MPI data type for every 3rd element
    MPI_Datatype vector_type;
    MPI_Type_vector(num_blocks, 1, stride, MPI_INT, &vector_type);
    MPI_Type_commit(&vector_type);

    // Exchange every 3rd element between the two processes
    if (world_rank == 0) {
        // Process 0 sends data to Process 1
        MPI_Send(data.data(), 1, vector_type, 1, 0, MPI_COMM_WORLD);
        // Process 0 receives data from Process 1
        MPI_Recv(data.data(), 1, vector_type, 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    } else if (world_rank == 1) {
        // Process 1 receives data from Process 0
        MPI_Recv(data.data(), 1, vector_type, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        // Process 1 sends data to Process 0
        MPI_Send(data.data(), 1, vector_type, 0, 0, MPI_COMM_WORLD);
    }

    // Handle remainder (leftover elements after last full block)
    if (remainder > 0) {
        if (world_rank == 0) {
            MPI_Send(&data[array_size - remainder], remainder, MPI_INT, 1, 1, MPI_COMM_WORLD);
            MPI_Recv(&data[array_size - remainder], remainder, MPI_INT, 1, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        } else if (world_rank == 1) {
            MPI_Recv(&data[array_size - remainder], remainder, MPI_INT, 0, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            MPI_Send(&data[array_size - remainder], remainder, MPI_INT, 0, 1, MPI_COMM_WORLD);
        }
    }

    // Print the data after communication
    std::cout << "Process " << world_rank << " received data: ";
    for (int i = 0; i < array_size; ++i) {
        std::cout << data[i] << " ";
    }
    std::cout << std::endl;

    // Cleanup
    MPI_Type_free(&vector_type);
    MPI_Finalize();
    return 0;
}

