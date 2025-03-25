#include <iostream>
#include <mpi.h>
#include <fstream>
#include <vector>

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);

    int world_rank, world_size;
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);
    
    // Eliminar el archivo al inicio si es el proceso 0
    if (world_rank == 0) {
        std::remove("ring.txt"); // Borra el archivo si existe
    }

    // Asegurar que el proceso 0 haya eliminado el archivo antes de que los demás procesos escriban
    MPI_Barrier(MPI_COMM_WORLD); 

    int prev_rank = (world_rank - 1 + world_size) % world_size;
    int next_rank = (world_rank + 1) % world_size;

    // Definir el tamaño del vector
    const int vec_size = 4;
    std::vector<int> world_vector(vec_size, world_rank);

    // Enviar y recibir el vector
    MPI_Send(world_vector.data(), vec_size, MPI_INT, next_rank, 0, MPI_COMM_WORLD);
    MPI_Recv(world_vector.data(), vec_size, MPI_INT, prev_rank, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

    // Imprimir el resultado en consola
    std::cout << "Process " << world_rank << " sent the vector [";
    for (int i = 0; i < vec_size; ++i) {
        std::cout << world_vector[i];
        if (i < vec_size - 1) std::cout << ", ";
    }
    std::cout << "] to " << next_rank << std::endl;

    // Guardar el resultado en el archivo
    std::ofstream file("ring.txt", std::ios::app);
    file << "Process " << world_rank << " sent the vector [";
    for (int i = 0; i < vec_size; ++i) {
        file << world_vector[i];
        if (i < vec_size - 1) file << ", ";
    }
    file << "] to " << next_rank << std::endl;
    file.close();

    MPI_Finalize();

    return 0;
}

