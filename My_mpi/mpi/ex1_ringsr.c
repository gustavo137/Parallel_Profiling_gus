#include <stdio.h>
#include <mpi.h>
#include <stdlib.h>

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);

    int rank, value, nps;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &nps);

    int prev_rank = (rank - 1 + nps) % nps;
    int next_rank = (rank + 1) % nps;

    value = rank;  // value to send

    char snum[5];  // rank into string 
    sprintf(snum, "%d", rank);

    char name[15];
    sprintf(name, "file%s.gtxt", snum);

    FILE *fp;
    fp = fopen(name, "a");

    for (int i = 0; i < nps; ++i) {
        int received_value;

        // Using MPI_Sendrecv to avoid send and recive at the same time, avoiding  deadlock
        MPI_Sendrecv(&value, 1, MPI_INT, next_rank, 0,
                     &received_value, 1, MPI_INT, prev_rank, 0,
                     MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        // save the recv value in the file
        // this form sabe the ring in counterclockwise "sentido antihorario"
        //fprintf(fp, "%d", received_value);
        
        // this form is clockwise "sentido horario "
        fprintf(fp, "%d -> ", (received_value+(2*i+1))%nps);

        //new value to send 
        value = received_value;
    }

    fclose(fp);
    MPI_Finalize();
    return 0;
}

