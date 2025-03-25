#include <iostream>
#include <mpi.h>

int main(int argc, char **argv) {
  MPI_Init(&argc, &argv);
  int world_size, world_rank;

  MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
  MPI_Comm_size(MPI_COMM_WORLD, &world_size);

  int color = world_rank % 3;
  MPI_Comm new_comm;
  MPI_Comm_split(MPI_COMM_WORLD, color, world_rank, &new_comm);

  // Get the new rank and size in the split communicator
  int new_rank, new_size;
  MPI_Comm_rank(new_comm, &new_rank);
  MPI_Comm_size(new_comm, &new_size);

  // get the new root of every group
  // Determine the root for broadcasting within each communicator
  //(rank 0  in each group for simpliciti)
  int root = 0;
  int value;

  if (new_rank == root) {
    // if itś the root process in this communicator, assing a value
    // to broadcast
    value = world_rank; // can be whatever but por learning propuses is this
    std::cout << "Root process " << world_rank
              << " is broadcasting the value: " << value << " in communicator "
              << color << std::endl;
  }
  // Broadcast the value from the root procces to all the group in the
  // communicator
  MPI_Bcast(&value, 1, MPI_INT, root, new_comm);

  // See the comments at the end 1) and 2) 
  // 1) explain the "problem" (is not aproblem)
  // 2) solution for learning porposes
  // Synchronize all processes before printing
  MPI_Barrier(MPI_COMM_WORLD);
  
  // then all processes print the result after receibing the broadcast value
  std::cout << "Process " << world_rank << " (in communicator " << color
            << ") received value: " << value << std::endl;
  MPI_Comm_free(&new_comm);
  MPI_Finalize();
  return 0;
}
/*
1)  The processes' outputs might seem out of sequence. This is common because the different communicators run in parallel,
    and the timing of when each process prints its result isn't synchronized.
    Each communicator is broadcasting correctly, but since the printing is happening asynchronously, the order of
    the outputs is not linear. This behavior is expected in parallel computing environments like MPI.
2)  If you want the output to appear in order (rank 0 first, then rank 1, etc.), you could force the processes to synchronize
    before printing, using MPI_Barrier or ordering print statements carefully. However, this would be purely for clarity in
    viewing the output and doesn't affect the correctness of the broadcasts. 
*/

