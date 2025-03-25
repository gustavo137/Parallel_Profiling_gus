#include <mpi.h>
#include <iostream>

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);

    int world_rank, world_size;
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    // Ahora no limitamos a exactamente 2 procesos
    if (world_size < 2) {
        std::cerr << "This program requires at least 2 processes" << std::endl;
        MPI_Abort(MPI_COMM_WORLD, 1);
    }

    int send_data = world_rank;
    int recv_data = -1;

    MPI_Request request_send, request_recv;  // Para manejar comunicación no bloqueante

    // Definir el siguiente y anterior proceso en el anillo
    int next_rank = (world_rank + 1) % world_size;         // Próximo proceso en el anillo
    int prev_rank = (world_rank - 1 + world_size) % world_size;  // Proceso anterior en el anillo

    // Cada proceso envía al siguiente y recibe del anterior
    MPI_Isend(&send_data, 1, MPI_INT, next_rank, 0, MPI_COMM_WORLD, &request_send);
    MPI_Irecv(&recv_data, 1, MPI_INT, prev_rank, 0, MPI_COMM_WORLD, &request_recv);

    // Esperar a que ambos envíos y recepciones no bloqueantes se completen
    MPI_Wait(&request_send, MPI_STATUS_IGNORE);
    MPI_Wait(&request_recv, MPI_STATUS_IGNORE);

    std::cout << "Process " << world_rank << " received data: " << recv_data << " from process " << prev_rank << std::endl;

    MPI_Finalize();
    return 0;
}

