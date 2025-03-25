#include <mpi.h>
#include <iostream>

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);

    int world_rank, world_size;
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    if (world_size != 2) {
        std::cerr << "This program requires exactly 2 processes" << std::endl;
        MPI_Abort(MPI_COMM_WORLD, 1);
    }

    int send_data = world_rank;
    int recv_data = -1;

    MPI_Request request_send, request_recv;  // To handle non-blocking communication

    // Process 0 sends data to Process 1 and receives from Process 1
    if (world_rank == 0) {
        MPI_Isend(&send_data, 1, MPI_INT, 1, 0, MPI_COMM_WORLD, &request_send);  
        MPI_Irecv(&recv_data, 1, MPI_INT, 1, 0, MPI_COMM_WORLD, &request_recv); 
    }
    // Process 1 sends data to Process 0 and receives from Process 0
    else if (world_rank == 1) {
        MPI_Isend(&send_data, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, &request_send);  
        MPI_Irecv(&recv_data, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, &request_recv); 
    }

    // Wait for both non-blocking send and receive to complete
    //this is a very important part
    MPI_Wait(&request_send, MPI_STATUS_IGNORE);
    MPI_Wait(&request_recv, MPI_STATUS_IGNORE);

    std::cout << "Process " << world_rank << " received data: " << recv_data << std::endl;

    MPI_Finalize();
    return 0;
}
