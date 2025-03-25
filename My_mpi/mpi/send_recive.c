#include <mpi.h>
#include <iostream>
#include <vector>

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);

    int world_rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

    int world_size;
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    if (world_size < 2) {
        std::cerr << "World size must be at least two for this example" << std::endl;
        MPI_Abort(MPI_COMM_WORLD, 1);//this is how you "exit" from MPI program
    }

    std::vector<int> numbers;
    numbers.resize(10);
    if (world_rank == 0) {
        // Send the numbers vector from process 0 to process 1
        numbers[0] = 77;
        MPI_Send(numbers.data(), numbers.size(), MPI_INT, 1, 0, MPI_COMM_WORLD);
    } else if (world_rank == 1) {
        // Receive the numbers at process 1 from process 0
        MPI_Recv(numbers.data(), numbers.size(), MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        std::cout << "checking first number on process 1: " << numbers[0] << std::endl;
    }

    MPI_Finalize();
    return 0;
}
