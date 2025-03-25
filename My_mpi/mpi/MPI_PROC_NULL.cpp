#include <mpi.h>
#include <iostream>

/*
 You can send to (or recieve from) "nothing". In this case the operation just won't happen,
 but you can write more genral code and avoid extra "ifs". You will see a useful example in
 the next course about mpi, but so far we want to see how it works and remember it exists.
*/
int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);

    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    int world_size;
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    int send_data{rank};
    int recv_data{777};

    // Example: Send to the next process, receive from the previous process
    //without "periodic boundary conditions" 
    int next_rank = (rank == world_size - 1) ? MPI_PROC_NULL : rank + 1;
    int prev_rank = (rank == 0) ? MPI_PROC_NULL : rank - 1;

    MPI_Sendrecv(&send_data, 1, MPI_INT, next_rank, 0,  // Send to next process
                 &recv_data, 1, MPI_INT, prev_rank, 0,  // Receive from previous process
                 MPI_COMM_WORLD, MPI_STATUS_IGNORE);

    std::cout << "Process " << rank << " sent " << send_data
              << " to process " << next_rank << " and received " << recv_data
              << " from process " << prev_rank << std::endl;

    MPI_Finalize();
    return 0;
}
