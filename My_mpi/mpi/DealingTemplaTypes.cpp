#include <mpi.h>
#include <iostream>
#include <cstring>
#include <vector>

struct CustomData {
    int id;
    double value;
};

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);

    int world_rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

    CustomData data;
    std::vector<int> vec;
    vec.resize(10);
 
    if (world_rank == 0) {
        // Process 0 sends CustomData
        data.id = 123;
        data.value = 456.789;
        MPI_Send(&data, sizeof(CustomData), MPI_BYTE, 1, 0, MPI_COMM_WORLD);
    } else if (world_rank == 1) {
        // Process 1 receives CustomData
        MPI_Recv(&data, sizeof(CustomData), MPI_BYTE, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        std::cout << "Process 1 received id: " << data.id << ", value: " << data.value << std::endl;
    }

    if (world_rank == 0) {
        vec[0]=99;
     //here we know it's ints, but lets pretend we don't
        MPI_Send(static_cast<void*>(vec.data()), vec.size()*sizeof(int), MPI_BYTE, 1, 0, MPI_COMM_WORLD);
    } else if (world_rank == 1) {
        MPI_Recv(static_cast<void*>(vec.data()), vec.size()*sizeof(int), MPI_BYTE, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        std::cout << "Process 1: " << vec[0] << std::endl;
    } 

    MPI_Finalize();
    return 0;
}
