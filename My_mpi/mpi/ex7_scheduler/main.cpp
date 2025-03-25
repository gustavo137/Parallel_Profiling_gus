#include <mpi.h>
#include <iostream>
#include <vector>
#include <random>
#include <algorithm>
#include <unistd.h> // for sleep

const int WORKTAG = 0;
const int STOPTAG = 1;
const int NUM_TASKS = 10; // number of tasks

// Generate the random times of sleep between 1 and 10 Genera
void generate_sleep_times(std::vector<int>& sleep_times, int num_tasks) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(1, 10);
    sleep_times.resize(num_tasks);
    std::generate(sleep_times.begin(), sleep_times.end(), [&]() { return dis(gen); });
}

//Function to supervise the tasks and give tasks
void supervise_work(const std::vector<int>& sleep_times, int num_workers) {
    int num_tasks = sleep_times.size();
    int task_index = 0;
    MPI_Status status;

    //Only set task to the needed processors
    ////if there are more workers that task only the same number of tasts and workers works
    int active_workers = std::min(num_tasks, num_workers - 1);
    for (int i = 1; i <= active_workers && task_index < num_tasks; ++i) {
        MPI_Send(&sleep_times[task_index], 1, MPI_INT, i, WORKTAG, MPI_COMM_WORLD);
        task_index++;
    }

    //Asing more task to the workers who finished
    while (task_index < num_tasks) {
        MPI_Recv(NULL, 0, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
        int worker_rank = status.MPI_SOURCE;
        MPI_Send(&sleep_times[task_index], 1, MPI_INT, worker_rank, WORKTAG, MPI_COMM_WORLD);
        task_index++;
    }

    //Send stop signals to the active workers
    for (int i = 1; i <= active_workers; ++i) {
        MPI_Recv(NULL, 0, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
        int worker_rank = status.MPI_SOURCE;
        MPI_Send(NULL, 0, MPI_INT, worker_rank, STOPTAG, MPI_COMM_WORLD);
    }

    // Tell to the rest of the workers that does not have work to stop
    for (int i = active_workers + 1; i < num_workers; ++i) {
        MPI_Send(NULL, 0, MPI_INT, i, STOPTAG, MPI_COMM_WORLD);
    }
}
// Function work to recive and do the tasks
void do_work() {
    MPI_Status status;
    int sleep_time;

    while (true) {
        // recive the task or stop signal
        MPI_Recv(&sleep_time, 1, MPI_INT, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

        // if we recive the STOPTAG, then the rank finished the taks
        if (status.MPI_TAG == STOPTAG) {
            int rank;
            MPI_Comm_rank(MPI_COMM_WORLD, &rank);
           // std::cout << "Process " << rank << " received stop signal and is exiting.\n";
            break;
        }

        // do the task sleep, for the time in sleep_time
        int rank;
        MPI_Comm_rank(MPI_COMM_WORLD, &rank);
        std::cout << "Process " << rank << " sleeping for " << sleep_time << " seconds.\n";
        sleep(sleep_time);
        std::cout << "Process " << rank << " finished the task (has woken up).\n";
        
        //Send to the master that the rank has been finished the  task
        // Notificar al maestro que el proceso ha terminado su tarea
        MPI_Send(NULL, 0, MPI_INT, 0, 0, MPI_COMM_WORLD);
    }
}

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);


    if (size < 2) {
        std::cerr << "The number of processors needs to be > 1 for " << argv[0] << std::endl;
        MPI_Abort(MPI_COMM_WORLD, 1);
    }

    if (rank == 0) {
        // master or scheduler
        std::vector<int> sleep_times;
        generate_sleep_times(sleep_times, NUM_TASKS);
        supervise_work(sleep_times, size);
    } else {
        // the rest of the ranks will work 
        do_work();
    }

    MPI_Finalize();
    return 0;
}

