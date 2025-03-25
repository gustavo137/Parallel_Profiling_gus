#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <omp.h>
#include <time.h>

#define SEED    918273
#define MIN(X, Y) (((X) < (Y)) ? (X) : (Y))

#ifdef USE_FLOAT
    typedef float  Float_t;
#else
    typedef double Float_t;
#endif


void mmp_serial(const Float_t * restrict A, const Float_t * restrict B, Float_t * restrict C,
	          const long long unsigned int N, const long long unsigned int L, const long long unsigned int M) {
  
  for (long long unsigned int i = 0; i < N; ++i) {
    for (long long unsigned int j = 0; j < M; ++j) {
      for (long long unsigned int k = 0; k < L; ++k) {
         C[i * M + j] += A[i * L + k] * B[k * M + j];
      }
    }
  }   
}

void mmp_parallel(const Float_t * restrict A, const Float_t * restrict B, Float_t * restrict C,
	          const long long unsigned int N, const long long unsigned int L, const long long unsigned int M) {
  
  #pragma omp parallel for collapse(2)
  for (long long unsigned int i = 0; i < N; ++i) {
    for (long long unsigned int j = 0; j < M; ++j) {
      Float_t sum = 0;
      #pragma omp simd reduction(+:sum) 
      for (long long unsigned int k = 0; k < L; ++k) {
	 sum += A[i * L + k] * B[k * M +  j];
      }
      C[i * M + j] = sum;
    }
  }   
}

void mmp_parallel_blocks(const Float_t * restrict A, const Float_t * restrict B, Float_t * restrict C,
                  const long long unsigned int N, const long long unsigned int L, const long long unsigned int M) {

  Float_t * D  = malloc(sizeof(Float_t) * M * L);
  #pragma omp parallel for collapse(2)
  for (long long unsigned int i = 0; i < M; ++i) {
    for (long long unsigned int j = 0; j < L; ++j) {
      D[i * L + j] = B[j * M + i];
    }
  }

  int Ar_blocksize = MIN(N,32);
  int Bc_blocksize = MIN(M,32);
  long long unsigned int Ar_N = N / Ar_blocksize + N % Ar_blocksize;
  long long unsigned int Bc_N = M / Bc_blocksize + M % Bc_blocksize;
  #pragma omp parallel for collapse(2)
  for (long long unsigned int ii = 0; ii < Ar_N; ++ii) {
    for (long long unsigned int jj = 0; jj < Bc_N; ++jj) {
      long long unsigned int i_start = ii * Ar_blocksize;
      long long unsigned int i_end = i_start + Ar_blocksize;
      i_end = (i_end > N ? N : i_end);
      for (long long unsigned int i = i_start; i < i_end; ++i) {
        long long unsigned int j_start = jj * Bc_blocksize;
        long long unsigned int j_end   = j_start + Bc_blocksize;
        j_end = (j_end > M ? M : j_end);
        for (long long unsigned int j = j_start; j < j_end; ++j) {
          Float_t sum = 0;
          #pragma omp simd reduction(+:sum)  
          for (long long unsigned int k = 0; k < L; ++k) {  
            sum += A[i * L + k] * D[j * L +  k];
	  }
          C[i * M + j] = sum;
        }
      }
    }
  }
  free(D);
}

void mmp_parallel_tiled(const Float_t * restrict A, const Float_t * restrict B, Float_t * restrict C,
                  const long long unsigned int N, const long long unsigned int L, const long long unsigned int M) {
  int Ar_blocksize = MIN(N,32);
  int Ac_blocksize = MIN(L,32);
  int Bc_blocksize = MIN(M,32);
  long long unsigned int Ar_N = N / Ar_blocksize + N % Ar_blocksize;
  long long unsigned int Ac_N = L / Ac_blocksize + L % Ac_blocksize;
  long long unsigned int Bc_N = M / Bc_blocksize + M % Bc_blocksize;
  #pragma omp parallel for collapse(2)
  for (long long unsigned int ii = 0; ii < Ar_N; ++ii) {
    for (long long unsigned int jj = 0; jj < Bc_N; ++jj) {
      for (long long unsigned int kk = 0; kk < Ac_N; ++kk) {
        long long unsigned int i_start = ii * Ar_blocksize;
        long long unsigned int i_end = i_start + Ar_blocksize;
        i_end = (i_end > N ? N : i_end);
        for (long long unsigned int i = i_start; i < i_end; ++i) {
          long long unsigned int j_start = jj * Bc_blocksize;
          long long unsigned int j_end   = j_start + Bc_blocksize;
          j_end = (j_end > M ? M : j_end);
          for (long long unsigned int j = j_start; j < j_end; ++j) {
            long long unsigned int k_start = kk * Ac_blocksize;
            long long unsigned int k_end   = k_start + Ac_blocksize;
            k_end = (k_end > L ? L : k_end);
            for (long long unsigned int k = k_start; k < k_end; ++k) {
              C[i * M + j] += A[i * L + k] * B[k * M +  j];
	    }
          }
        }
      }
    }
  }
}



int check(const Float_t * restrict A, const Float_t * restrict B, const long long unsigned int N, const long long unsigned int M) {
  int errs = 0;
#ifdef USE_FLOAT
  const Float_t tol = 1e-2;
#else
  const Float_t tol = 1e-5;
#endif
  long long unsigned int Dim = N * M;
  #pragma omp parallel for reduction(+:errs) 
  for ( long long unsigned int ii = 0; ii < Dim; ++ii) {
    if ( fabs( A[ii] - B[ii] ) > tol )
      ++errs;
  }
  return errs;
}


int main () {
  const long long unsigned int n = 1024;
  const long long unsigned int m = 1024;
  const long long unsigned int l = 1024;
  Float_t * a  = malloc(sizeof(Float_t) * n * l);  
  Float_t * b  = malloc(sizeof(Float_t) * l * m);  
  Float_t * c0 = malloc(sizeof(Float_t) * n * m); 
  Float_t * c1 = malloc(sizeof(Float_t) * n * m); 
  Float_t * c2 = malloc(sizeof(Float_t) * n * m); 
  Float_t * c3 = malloc(sizeof(Float_t) * n * m); 
  int nthreads;
  #pragma omp parallel 
  #pragma omp master
    nthreads = omp_get_num_threads();   ; 
  printf("Number of Threads: %d\n", nthreads);
  #pragma omp parallel
  {
    int myid = omp_get_thread_num();
    int unsigned short myseeds[3] = {SEED+(myid),SEED+(myid*3+1), SEED+(myid*4+2)};
    seed48( myseeds );
    #pragma omp for 
    for(long long unsigned int i = 0; i < n * l; i++)
      a[i] = erand48( myseeds ); 
    #pragma omp for 
    for(long long unsigned int i = 0; i < m * l; i++)
      b[i] = erand48( myseeds ); 
    #pragma omp for 
    for(long long unsigned int i = 0; i < n * m; i++) {
      c0[i] = 0.;
      c1[i] = 0.;
      c2[i] = 0.;
      c3[i] = 0.;
    }
  }
  /*
  double t0 = omp_get_wtime();
  mmp_serial(a,b,c0,n,l,m);
  t0 = omp_get_wtime() - t0;

  double t1 = omp_get_wtime();
  mmp_parallel(a,b,c1,n,l,m);
  t1 = omp_get_wtime() - t1;
  
  double t2 = omp_get_wtime();
  mmp_parallel_blocks(a,b,c2,n,l,m);
  t2 = omp_get_wtime() - t2;
*/
  double t3 = omp_get_wtime();
  mmp_parallel_tiled(a,b,c3,n,l,m);
  t3 = omp_get_wtime() - t3;
  /*
  int v01 = check(c0, c1, n, m);
  int v02 = check(c0, c2, n, m);
  int v03 = check(c0, c3, n, m);
  int v12 = check(c1, c2, n, m);
  int v13 = check(c1, c3, n, m);
  int v23 = check(c2, c3, n, m);
  printf("t0 == %f\nt1 == %f\nt2 == %f\nt3 == %f\n", t0, t1, t2, t3);
  printf("c0 == c1? %s\n", v01? "false" : "true");
  printf("c0 == c2? %s\n", v02? "false" : "true");
  printf("c0 == c3? %s\n", v03? "false" : "true");
  printf("c1 == c2? %s\n", v12? "false" : "true");
  printf("c1 == c3? %s\n", v13? "false" : "true");
  printf("c2 == c3? %s\n", v23? "false" : "true");
  */
  free(a); free(b); free(c0); free(c1); free(c2); free(c3);
  return 0;
}

