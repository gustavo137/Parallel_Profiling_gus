#include <iostream>
#include <mpi.h>
#include <fstream>
#include <vector>
#include"Parallel_Timer.hpp"
int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);

    int world_rank, world_size;
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

{CParallel_Timer t("sum Allreduce TIMING");

    // Eliminar el archivo al inicio si es el proceso 0
    if (world_rank == 0) {
        std::remove("Allreduce.txg");
    }

    MPI_Barrier(MPI_COMM_WORLD); // Sincronizar para asegurarse de que el archivo esté eliminado


    // Definir el tamaño del vector
    const int vec_size = 4;
    std::vector<int> world_vector(vec_size, world_rank);  // Inicializar el vector con el valor de world_rank
    std::vector<int> sum_vector(world_vector);  // Vector para acumular la suma

    // Guardar el resultado inicial en el archivo
    std::ofstream file("Allreduce.txg", std::ios::app);
    file << "Process " << world_rank << " started with vector [";
    for (int i = 0; i < vec_size; ++i) {
        file << world_vector[i];
        if (i < vec_size - 1) file << ", ";
    }
    file << "]\n";
    file.close();
////////////
    MPI_Allreduce(world_vector.data(),sum_vector.data(),vec_size,MPI_INT,MPI_SUM,MPI_COMM_WORLD);
//////
    // Guardar el vector resultante después de la suma en el archivo
    file.open("Allreduce.txg", std::ios::app);
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

