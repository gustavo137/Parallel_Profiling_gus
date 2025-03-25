#include <iostream>
#include <mpi.h>
#include <fstream>
#include <vector>
#include "Parallel_Timer.hpp"

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);

    int world_rank, world_size;
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);
 
{CParallel_Timer t("ring Isend and Irecv TIMING");
   
    // Eliminar el archivo al inicio si es el proceso 0
    if (world_rank == 0) {
        std::remove("ringIsr.txg");
    }

    MPI_Barrier(MPI_COMM_WORLD); // Sincronizar para asegurarse de que el archivo esté eliminado

    int prev_rank = (world_rank - 1 + world_size) % world_size;
    int next_rank = (world_rank + 1) % world_size;

    // Definir el tamaño del vector
    const int vec_size = 50000;
    std::vector<int> world_vector(vec_size, world_rank);  // Inicializar el vector con el valor de world_rank
    std::vector<int> recv_vector(vec_size, 0);  // Vector para recibir datos
    std::vector<int> sum_vector(world_vector);  // Vector para acumular la suma

    // Guardar el resultado inicial en el archivo
    std::ofstream file("ringIsr.txg", std::ios::app);
    file << "Process " << world_rank << " started with vector [";
    for (int i = 0; i < vec_size; ++i) {
        file << world_vector[i];
        if (i < vec_size - 1) file << ", ";
    }
    file << "]\n";
    file.close();

    // Comunicación en anillo
    for (int step = 0; step < world_size - 1; ++step) {
        MPI_Request request_send, request_recv;  // To handle non-blocking communication
        // Enviar el vector actual al siguiente proceso y recibir del anterior
        /*
        MPI_Sendrecv(world_vector.data(), vec_size, MPI_INT, next_rank, 0,
                     recv_vector.data(), vec_size, MPI_INT, prev_rank, 0,
                     MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        */
        MPI_Irecv(recv_vector.data(),vec_size,MPI_INT,prev_rank,0,MPI_COMM_WORLD, &request_recv);
       
        MPI_Isend(world_vector.data(),vec_size,MPI_INT,next_rank,0,MPI_COMM_WORLD, &request_send);
        MPI_Wait(&request_recv, MPI_STATUS_IGNORE);       
        //to overlap computation and communication.
        // Sumar el vector recibido al vector suma (acumulador)
        for (int i = 0; i < vec_size; ++i) {
            sum_vector[i] += recv_vector[i];
        }
        
        
        // Wait for both non-blocking send and receive to complete
        //this is a very important part
        MPI_Wait(&request_send, MPI_STATUS_IGNORE);
        
        /* 
        // Sumar el vector recibido al vector suma (acumulador)
        for (int i = 0; i < vec_size; ++i) {
            sum_vector[i] += recv_vector[i];
        }
        */
        
        // Ahora el vector recibido se convierte en el vector para enviar en la siguiente iteración
        world_vector.swap(recv_vector);
    }

    // Guardar el vector resultante después de la suma en el archivo
    file.open("ringIsr.txg", std::ios::app);
    file << "Process " << world_rank << " has final vector [";
    for (int i = 0; i < vec_size; ++i) {
        file << sum_vector[i];
        if (i < vec_size - 1) file << ", ";
    }
    file << "]\n";
    file.close();
}
   CParallel_Timer::gather_and_process_times(world_rank, world_size);
    // Finalizar MPI
    MPI_Finalize();

    return 0;
}

