#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <mpi.h>

// Function to generate random numbers
std::vector<float> create_rand_nums(int num_elements) {
    std::vector<float> rand_nums(num_elements);
    for (int i = 0; i < num_elements; ++i) {
        rand_nums[i] = static_cast<float>(rand()) / RAND_MAX; // Generate a random float between 0 and 1
    }
    return rand_nums;
}

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);

    int world_rank, world_size;
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    int num_elements_per_proc = 10; // Number of elements each process will handle
    std::vector<float> rand_nums;

    // Generate random numbers on the root process (process 0)
    if (world_rank == 0) {
        srand(static_cast<unsigned>(time(0))); // Seed the random number generator
        rand_nums = create_rand_nums(num_elements_per_proc * world_size);
    }

    // Scatter the random numbers to all processes
    std::vector<float> local_nums(num_elements_per_proc);
    MPI_Scatter(rand_nums.data(), num_elements_per_proc, MPI_FLOAT,
                local_nums.data(), num_elements_per_proc, MPI_FLOAT,
                0, MPI_COMM_WORLD);

    // Calculate the local sum
    float local_sum = 0.0;
    for (float num : local_nums) {
        local_sum += num;
    }

    // Reduce all of the local sums into the global sum at the root process
    float global_sum;
    MPI_Reduce(&local_sum, &global_sum, 1, MPI_FLOAT, MPI_SUM, 0, MPI_COMM_WORLD);

    // Print the result on the root process
    if (world_rank == 0) {
        std::cout << "Total sum = " << global_sum << ", "
                  << "Average = " << global_sum / (world_size * num_elements_per_proc) << std::endl;
    }

    MPI_Finalize();
    return 0;
}

