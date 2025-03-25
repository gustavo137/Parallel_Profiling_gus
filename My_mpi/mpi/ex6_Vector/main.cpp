#include "Parallel_Timer.hpp"
#include "vector_operations.hpp"

int main(int argc, char **argv) {
  MPI_Init(&argc, &argv);

  // Get the size of wold and the number of ranks
  int world_size;
  MPI_Comm_size(MPI_COMM_WORLD, &world_size);
  int rank;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  // Size of the matrix N*N
  int N = 10;

  // create the objet CMatrix
  CMatrix<int> A(N, rank, world_size, MPI_COMM_WORLD);

  // Size of the matriz
  int total_elements = N * N;

  // Get the size and range that each process fill
  int elements_per_proc = total_elements / world_size; // floor function
  int remainder = total_elements % world_size;
  int start_idx = rank * elements_per_proc;
  int end_idx = start_idx + elements_per_proc;

  // The las process fill the remainder elements
  if (rank == world_size - 1) {
    end_idx += remainder;
  }

  // fill only the part for each processor
  A.fill(start_idx, end_idx);

  // each processor write its work in a file with its name
  char snum[5];
  sprintf(snum, "%d", A.rank);
  char name[15];
  sprintf(name, "rank%s.txg", snum); // txt -> txg only to add txg to .gitignore
  FILE *fp;
  fp = fopen(name, "w");

  {
    CParallel_Timer t("Timing the save data for each processor");
    // Save its oun data
    for (int i = start_idx; i < end_idx; ++i) {
      fprintf(fp, "%d ", A.data[i]);
    }

    fclose(fp);

    // Thing to use  Gatherv
    std::vector<int> recvcounts(world_size); // Sizes received by each processor
    std::vector<int> displs(
        world_size); // strides (displacements) of each process

    // get recvcounts and displs only in root
    if (rank == 0) {
      for (int i = 0; i < world_size; ++i) {
        recvcounts[i] = (i == world_size - 1) ? (elements_per_proc + remainder)
                                              : elements_per_proc;
        displs[i] = (i == 0) ? 0 : displs[i - 1] + recvcounts[i - 1];
      }
    }

    ////
    // Gather the data in root (rank 0)
    MPI_Gatherv(&A.data[start_idx], end_idx - start_idx, MPI_INT, A.data.data(),
                recvcounts.data(), displs.data(), MPI_INT, 0, MPI_COMM_WORLD);

    // Root print the full vector/matrix
    if (rank == 0) {
      FILE *root_fp = fopen("rank0_full_matrix.txg", "w");
      if (root_fp == NULL) {
        std::cerr << "Error opening file for process 0" << std::endl;
      } else {
        fprintf(root_fp, "The entire matrix is:\n");
        /////to ptint as a matrix /////

        for (int i = 0; i < total_elements; ++i) {
          fprintf(root_fp, "%d ", A.data[i]);
          if ((i + 1) % N == 0)
            fprintf(
                root_fp,
                "\n"); // new line to see the data as a matrix, each N elemets
        }

        /// end print as matrix
        //////print as a vector
        /*
          for (const auto& val : A.data) {
             fprintf(root_fp, "%d ", val);
         }
         fprintf(root_fp, "\n");
         */
        /// end print as a vector
        //// close the file
        fclose(root_fp);
      }
    }
  }
  CParallel_Timer::gather_and_process_times(rank, world_size);
  MPI_Finalize();
  return 0;
}
