#include "Parallel_Timer.hpp"
#include <mpi.h>
#include <vector>
#include <iostream>
#include <fstream>

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);

    int world_rank, world_size;
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    //open file append add "std::ios::out | std::ios::app"
    std::ofstream file("time3.txt",std::ios::out);
    //send the std::cout to the file
    //save the origional buffer of std::cout 
    std::streambuf* coutbuf = std::cout.rdbuf();
    std::cout.rdbuf(file.rdbuf()); //redirects std::cout tothe file 

    const size_t vec_size = (2L * 1024L * 1024L * 1024L) / sizeof(double); // Vector de aproximadamente 2 GB
    //const size_t vec_size = (100L * 1024L * 1024L) / sizeof(double); // small vector to taste the code
    std::vector<double> large_vec(vec_size, 1.0);
    std::vector<double> recv_vec(vec_size, 0.0);

    MPI_Win win;
    MPI_Win_create(recv_vec.data(), recv_vec.size() * sizeof(double), sizeof(double), MPI_INFO_NULL, MPI_COMM_WORLD, &win);

//////////////////////// Send using MPI_Send
    {
        CParallel_Timer t("Sending vector using MPI_Send");
        if (world_rank == 0) {
            MPI_Send(large_vec.data(), large_vec.size(), MPI_DOUBLE, 1, 0, MPI_COMM_WORLD);
        } else if (world_rank == 1) {
            MPI_Recv(recv_vec.data(), recv_vec.size(), MPI_DOUBLE, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        }
    }
//////////////////////// Using MPI_Put
    {
        CParallel_Timer t("Sending vector using MPI_Put (one-sided communication)");
        if (world_rank == 0) {
            MPI_Win_lock(MPI_LOCK_EXCLUSIVE, 1, 0, win);
            MPI_Put(large_vec.data(), large_vec.size(), MPI_DOUBLE, 1, 0, large_vec.size(), MPI_DOUBLE, win);
            MPI_Win_unlock(1, win);
        }
        MPI_Barrier(MPI_COMM_WORLD);  // Sincronización
    }

    // Gather the times
    CParallel_Timer::gather_and_process_times(world_rank, world_size);
    //Restart std::cout to original flow 
    std::cout.rdbuf(coutbuf);
    file.close();

    MPI_Win_free(&win);
    MPI_Finalize();
    return 0;
}

