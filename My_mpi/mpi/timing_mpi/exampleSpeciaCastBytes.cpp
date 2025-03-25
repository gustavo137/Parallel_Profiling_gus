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

    //const size_t vec_size = (2L * 1024L * 1024L * 1024L) / sizeof(double); // Vector de aproximadamente 2 GB
    const size_t vec_size = (100L * 1024L * 1024L) / sizeof(double); // small vector to taste the code
    std::vector<double> large_vec(vec_size, 1.0);

    //open file append add "std::ios::out | std::ios::app"
    std::ofstream file("time21.txt",std::ios::out);    
    //send the std::cout to the file
    //save the origional buffer of std::cout 
    std::streambuf* coutbuf = std::cout.rdbuf();
    std::cout.rdbuf(file.rdbuf()); //redirects std::cout tothe file 


//////////////////////// use MPI specialization templates to send the vector time21.txt
    {
        CParallel_Timer t("Sending double vector using template specialization");
        if (world_rank == 0) {
            MPI_Send(large_vec.data(), large_vec.size(), MPI_DOUBLE, 1, 0, MPI_COMM_WORLD);
        } else if (world_rank == 1) {
            MPI_Recv(large_vec.data(), large_vec.size(), MPI_DOUBLE, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        }
    }

////////////////////////// send the vector casting to bytes time22.txt
/*    {
        CParallel_Timer t("Sending double vector as bytes");
        if (world_rank == 0) {
            MPI_Send(reinterpret_cast<void*>(large_vec.data()), large_vec.size() * sizeof(double), MPI_BYTE, 1, 0, MPI_COMM_WORLD);
        } else if (world_rank == 1) {
            MPI_Recv(reinterpret_cast<void*>(large_vec.data()), large_vec.size() * sizeof(double), MPI_BYTE, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        }
    }
  */  
///////////////////////////////
    // call the timer conclutions
    CParallel_Timer::gather_and_process_times(world_rank, world_size);
    //Restart std::cout to original flow 
    std::cout.rdbuf(coutbuf);
    file.close();

    MPI_Finalize();
    return 0;
}

