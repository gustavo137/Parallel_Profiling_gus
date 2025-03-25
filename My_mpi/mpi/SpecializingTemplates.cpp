#include <mpi.h>
#include <iostream>
#include <type_traits>

template<typename T>
void mpi_send(const T& data, int dest, int tag, MPI_Comm comm) {
    if constexpr (std::is_same<T, int>::value) {
        MPI_Send(&data, 1, MPI_INT, dest, tag, comm);
    } else if constexpr (std::is_same<T, double>::value) {
        MPI_Send(&data, 1, MPI_DOUBLE, dest, tag, comm);
    } else {
        MPI_Send(&data, sizeof(T), MPI_BYTE, dest, tag, comm);
    }
}

template<typename T>
void mpi_recv(T& data, int source, int tag, MPI_Comm comm) {
    if constexpr (std::is_same<T, int>::value) {
        MPI_Recv(&data, 1, MPI_INT, source, tag, comm, MPI_STATUS_IGNORE);
    } else if constexpr (std::is_same<T, double>::value) {
        MPI_Recv(&data, 1, MPI_DOUBLE, source, tag, comm, MPI_STATUS_IGNORE);
    } else {
        MPI_Recv(&data, sizeof(T), MPI_BYTE, source, tag, comm, MPI_STATUS_IGNORE);
    }
}

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);

    int world_rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

    int world_size;
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    if (world_size < 2) {
        std::cerr << "World size must be at least two for this example" << std::endl;
        MPI_Abort(MPI_COMM_WORLD, 1);
    }

    if (world_rank == 0) {
        int number = 123;
        mpi_send(number, 1, 0, MPI_COMM_WORLD);

        double dvalue = 456.789;
        mpi_send(dvalue, 1, 0, MPI_COMM_WORLD);
    } else if (world_rank == 1) {
        int number;
        mpi_recv(number, 0, 0, MPI_COMM_WORLD);
        std::cout << "Process 1 received int: " << number << std::endl;

        double dvalue;
        mpi_recv(dvalue, 0, 0, MPI_COMM_WORLD);
        std::cout << "Process 1 received double: " << dvalue << std::endl;
    }

    MPI_Finalize();
    return 0;
}
