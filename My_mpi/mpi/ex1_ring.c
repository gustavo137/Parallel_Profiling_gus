#include <stdio.h>
#include <mpi.h>

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);

    int rank, world_value, nps;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &nps);

    int prev_rank = (rank - 1 + nps) % nps;
    int next_rank = (rank + 1) % nps;            
    
    world_value = rank;   
    
    //MPI_Send(&world_value, 1, MPI_INT, next_rank, 0, MPI_COMM_WORLD);
    //MPI_Recv(&world_value, 1, MPI_INT, prev_rank, 0, MPI_COMM_WORLD,
    //                                                 MPI_STATUS_IGNORE);
    MPI_Sendrecv(&world_value, 1, MPI_INT, next_rank, 0,
                     &world_value, 1, MPI_INT, prev_rank, 0,
                     MPI_COMM_WORLD, MPI_STATUS_IGNORE);

    //printf("[%d] %d ", rank, world_value);
    printf("Process %d send the value (received) %d to  %d\n", rank, world_value, next_rank);
    /*
    FILE *fp;
    fp = fopen("ring.txt", "a");
    fprintf(fp,"Process %d send the value %d to  %d\n", rank, world_value, next_rank);
    fclose(fp);  
    */  
    MPI_Finalize();
    return 0;
}
