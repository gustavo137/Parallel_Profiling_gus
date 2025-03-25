#include <iostream>
#include <random>
#include "mtl.hpp"
#include <omp.h>
#include <chrono>

double wtime() {
#ifdef _OPENMP
  return omp_get_wtime();
#else
  using clock = std::chrono::high_resolution_clock;
  auto time = clock::now();
  auto duration = std::chrono::duration<double>(time.time_since_epoch());
  return duration.count();
#endif
}


inline std::string strbool(const bool b) {
  return (b? "true" : "false");
}

int main() {
  
  constexpr auto n{1024};
  constexpr auto m{1024};
  constexpr auto l{1024};
  #ifdef USE_FLOAT
  mtl::Matrix<float> a{n, l};
  mtl::Matrix<float> b{l, m};
  #else
  mtl::Matrix<double> a{n, l};
  mtl::Matrix<double> b{l, m};
  #endif
    int nthreads;
  #pragma omp parallel
  #pragma omp master
    nthreads = omp_get_num_threads();   ;
    std::cout << "Number of Threads: " << nthreads << "\n";

  #pragma omp parallel
  {
    std::random_device rd;
    std::seed_seq seed{static_cast<int>(rd()), omp_get_thread_num()};
    std::mt19937 generator(seed);
    std::uniform_real_distribution<double> uniform(0.0, 1.0);
    #pragma omp for collapse(2)
    for (std::size_t r = 0; r < a.num_rows(); ++r)
      for (std::size_t c = 0; c < a.num_cols(); ++c)
        a(r,c) = uniform(generator);

    #pragma omp for collapse(2)
    for (std::size_t r = 0; r < b.num_rows(); ++r)
      for (std::size_t c = 0; c < b.num_cols(); ++c)
        b(r,c) = uniform(generator);
  }
  auto t0 = wtime();
  auto c0 = mmp_serial(a,b);
  t0 = wtime() - t0;
/*
  auto t1{omp_get_wtime()};
  auto c1 = mmp_parallel(a,b);
  t1 = wtime() - t1;

  auto t2{omp_get_wtime()};
  auto c2 = mmp_parallel_blocks(a,b);
  t2 = wtime() - t2;

  auto t3{omp_get_wtime()};
  auto c3 = mmp_parallel_tiled(a,b);
  t3 = wtime() - t3;
  
  std::cout << "t0 == " << t0 << "\nt1 == " << t1 << "\nt2 == " << t2 << "\nt3 == " << t3 << "\n";
  std::cout << "c0 == c1? " << strbool(c0==c1) << "\n";
  std::cout << "c0 == c2? " << strbool(c0==c2) << "\n";
  std::cout << "c0 == c3? " << strbool(c0==c3) << "\n";
  std::cout << "c1 == c2? " << strbool(c1==c2) << "\n";
  std::cout << "c1 == c3? " << strbool(c1==c3) << "\n";
  std::cout << "c2 == c3? " << strbool(c2==c3) << "\n";
*/
  return 0;
}


