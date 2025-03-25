 //To use the timer 
 /**
 *  MPI_Init(&argc, &argv);<----------------------------------------
 *  USE THIS AS
 *  {CParallel_Timer t("WHATEVER YOU ARE TIMING");
 *        THE CODE YOU ARE TIMING
 *    }
 * 
 *  THEN AT THE END OF MAIN PUT
 *  CParallel_Timer::gather_and_process_times(world_rank, world_size); 
 *  MPI_Finalize();<----------------------------------------  
 */

#include <mpi.h>
#include <iostream>
#include <chrono>
#include <vector>
#include <string>
#include <algorithm>
#include <thread> // para usar std::this_thread::sleep_for
#include <numeric> 

// for time units you can adjust this as needed, milliseconds, seconds etc. 
// if you change this remember to change the units [ms] in line 32, 59, 60 and 61
using time_units = std::chrono::milliseconds;

class CParallel_Timer {
public:
    CParallel_Timer(const std::string& what) : time_what(what) {
        start_time = std::chrono::high_resolution_clock::now();
    }

    ~CParallel_Timer() {
        auto end_time = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<time_units>(end_time - start_time);
        std::cout << "Duration for '" << time_what << "': " << duration.count() << " ms" << std::endl;

        // Save the spend time in a local vector 
        local_times.push_back(duration.count());
    }

    // This function is the most important, because is the parallelization 
    // Here we use Gather to join all the data in the root proccess
/// 
static void gather_and_process_times(int world_rank, int world_size) {
    std::vector<long> global_times;
    // Gather all the local times in the root processor 
    if (world_rank == 0) {
        global_times.resize(world_size);
    }
    //Every processor send its local time to root processor 
    MPI_Gather(&local_times[0], 1, MPI_LONG, &global_times[0], 1, MPI_LONG, 0, MPI_COMM_WORLD);

    if (world_rank == 0) {
        long min_time = global_times[0];
        long max_time = global_times[0];
        long sum_time = 0;

        for (long time : global_times) {
            if (time < min_time) {
                min_time = time;
            }
            if (time > max_time) {
                max_time = time;
            }
            sum_time += time;
        }

        double avg_time = static_cast<double>(sum_time) / world_size;

        std::cout << "Min time: " << min_time << " ms" << std::endl;
        std::cout << "Max time: " << max_time << " ms" << std::endl;
        std::cout << "Average time: " << avg_time << " ms" << std::endl;
    }
}

///
private:
    std::string time_what;
    std::chrono::high_resolution_clock::time_point start_time;
    static inline std::vector<long> local_times;
};

