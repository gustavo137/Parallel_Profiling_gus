#include <mpi.h>
#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>

// Function to create a vector of random floats
std::vector<float> create_rand_nums(int num_elements) {
    std::vector<float> rand_nums(num_elements);
    for (int i = 0; i < num_elements; i++) {
        rand_nums[i] = static_cast<float>(rand()) / RAND_MAX; // Random float between 0 and 1
    }
    return rand_nums;
}

// Function to compute the average of a vector
float compute_avg(const std::vector<float>& array) {
    float sum = 0.0;
    for (float num : array) {
        sum += num;
    }
    return sum / array.size();
}

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);

    int world_rank, world_size;
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    // Number of elements per process
    int elements_per_proc = 100;  // This can be adjusted as needed

    std::vector<float> rand_nums;
    if (world_rank == 0) {
        // Only the root process creates the random vector
        rand_nums = create_rand_nums(elements_per_proc * world_size);
        std::cout << "Root process generated " << elements_per_proc * world_size << " random numbers between 0 and 1." << std::endl;
    }

    // Create a vector that will hold a subset of the random numbers
    std::vector<float> sub_rand_nums(elements_per_proc);

    // Scatter the random numbers from the root process to all processes
    MPI_Scatter(rand_nums.data(), elements_per_proc, MPI_FLOAT, sub_rand_nums.data(),
                elements_per_proc, MPI_FLOAT, 0, MPI_COMM_WORLD);

    // Each process computes the average of its subset
    float sub_avg = compute_avg(sub_rand_nums);

    // Create a vector to hold the averages from all processes
    std::vector<float> all_avgs(world_size);

    // Use MPI_Allgather to gather the averages from all processes to all processes
    MPI_Allgather(&sub_avg, 1, MPI_FLOAT, all_avgs.data(), 1, MPI_FLOAT, MPI_COMM_WORLD);

    // Every process can now compute the total average of all numbers
    float total_avg = compute_avg(all_avgs);
    std::cout << "Process " << world_rank << " computed the total average: " << total_avg << std::endl;

    MPI_Finalize();
    return 0;
}

