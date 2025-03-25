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
    std::ofstream file("time1.txt",std::ios::out); 
    //send the std::cout to the file
    //save the origional buffer of std::cout 
    std::streambuf* coutbuf = std::cout.rdbuf();
    std::cout.rdbuf(file.rdbuf()); //redirects std::cout tothe file 

    // L = type long, 1024=2^10, #of bytes = 2 gb, 268 million double type elements
    const size_t vec_size = (2L * 1024L * 1024L * 1024L) / sizeof(double); // Vector of aprox ~ 2 GB
    //const size_t vec_size = (100L * 1024L * 1024L) / sizeof(double); // small vector to taste the code
    std::vector<double> large_vec(vec_size, 1.0);

    if (world_size < 2) {
        std::cerr << "This code need at least 2 processor." << std::endl;
        MPI_Finalize();
        return 1;
    }

///////////////////////////// Send the vector in only one line 
    { 
        CParallel_Timer t("Sending vector in one MPI_Send");
        if (world_rank == 0) {
            MPI_Send(large_vec.data(), large_vec.size(), MPI_DOUBLE, 1, 0, MPI_COMM_WORLD);
        } else if (world_rank == 1) {
            MPI_Recv(large_vec.data(), large_vec.size(), MPI_DOUBLE, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        }
    }

/////////////////////////////// Send lement by element
    {
        CParallel_Timer t("Sending vector element by element");
        if (world_rank == 0) {
            for (size_t i = 0; i < large_vec.size(); ++i) {
                MPI_Send(&large_vec[i], 1, MPI_DOUBLE, 1, 0, MPI_COMM_WORLD);
            }
        } else if (world_rank == 1) {
            for (size_t i = 0; i < large_vec.size(); ++i) {
                MPI_Recv(&large_vec[i], 1, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            }
        }
    }
////////////////////////////////
    // Collect and display the results
    CParallel_Timer::gather_and_process_times(world_rank, world_size);
    //Restart std::cout to original flow 
    std::cout.rdbuf(coutbuf);
    file.close();    


    MPI_Finalize();
    return 0;
}

