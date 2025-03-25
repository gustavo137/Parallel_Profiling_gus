#include <iostream>
#include <vector>
#include <cstdlib>
#include <cmath>
#include <mpi.h>

// Function to generate random numbers
std::vector<float> create_rand_nums(int num_elements) {
    std::vector<float> rand_nums(num_elements);
    for (int i = 0; i < num_elements; ++i) {
        rand_nums[i] = static_cast<float>(rand()) / RAND_MAX; // Generate random float between 0 and 1
    }
    return rand_nums;
}

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);

    int world_rank, world_size;
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    int num_elements_per_proc = 10; // Number of elements per process
    std::vector<float> rand_nums = create_rand_nums(num_elements_per_proc);

    // Sum the numbers locally
    float local_sum = 0.0;
    for (float num : rand_nums) {
        local_sum += num;
    }

    // Reduce all local sums to compute the global sum for the mean
    float global_sum;
    MPI_Allreduce(&local_sum, &global_sum, 1, MPI_FLOAT, MPI_SUM, MPI_COMM_WORLD);

    // Calculate the global mean
    float mean = global_sum / (num_elements_per_proc * world_size);

    // Compute the local sum of the squared differences from the mean
    float local_sq_diff = 0.0;
    for (float num : rand_nums) {
        local_sq_diff += (num - mean) * (num - mean);
    }

    // Reduce all local squared differences to compute the global sum of squared differences
    float global_sq_diff;
    MPI_Allreduce(&local_sq_diff, &global_sq_diff, 1, MPI_FLOAT, MPI_SUM, MPI_COMM_WORLD);

    // Compute the standard deviation
    float stddev = sqrt(global_sq_diff / (num_elements_per_proc * world_size));

    // Print the result (every process will print the same result)
    std::cout << "Process " << world_rank << ": Mean = " << mean
              << ", Standard deviation = " << stddev << std::endl;

    MPI_Finalize();
    return 0;
}

