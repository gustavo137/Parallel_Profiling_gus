#include <cmath>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <memory>
#include <vector>
#include <typeinfo>
#include <string>
#include <type_traits>
#include <random>
#include <algorithm>
#include <iterator>
#include <mpi.h>

template <typename T>
class CMatrix {
public:
  std::vector<T> data;
  size_t size;
  int world_size;
  int rank;
  MPI_Comm new_comm;
  CMatrix(const int &N);//:size(N),data(N*N){};
  CMatrix(const int &N, const int& rank, const int& world_size, MPI_Comm comm);
  void fill(int start_idx, int end_idx); // function fill
  //void fill(); create 
  //void print();
  // new operators
  template <typename U>
  friend std::ostream &operator<<(std::ostream &os, const CMatrix<U> &p);
  template <typename U>
  friend CMatrix<U> operator*(const CMatrix<U> &A, const CMatrix<U> &B);
  template <typename U>
  friend CMatrix<U> operator+(const CMatrix<U> &A, const CMatrix<U> &B);
};

////////
// Constructor
template <typename T>
CMatrix<T>::CMatrix(const int &N) : size(N), data(N * N) {
 data.reserve(N*N);//resize 
}

template <typename T>
CMatrix<T>::CMatrix(const int &N, const int& rank1, const int& world_size1, MPI_Comm comm) : size(N), data(N * N), rank(rank1), world_size(world_size1), new_comm(comm) {
 data.resize(N*N);//resize 
}
///////

// Destructor no more needed
// fill matrix depending on type
/* 
template <typename T>
void CMatrix<T>::fill(int start_idx, int end_idx) {
    if constexpr (std::is_same_v<int, T>) {
        std::mt19937 engine(std::random_device{}());
        std::uniform_int_distribution<int> dist(1, 10);
        for (int i = start_idx; i < end_idx; ++i) {
            data[i] = dist(engine);
        }
    }
    else if constexpr (std::is_same_v<double, T>) {
        std::mt19937 engine(std::random_device{}());
        std::uniform_real_distribution<double> dist(1.0, 10.0);
        for (int i = start_idx; i < end_idx; ++i) {
            data[i] = dist(engine);
        }
    }
    else if constexpr (std::is_same_v<float, T>) {
        std::mt19937 engine(std::random_device{}());
        std::uniform_real_distribution<float> dist(1.0f, 10.0f);
        for (int i = start_idx; i < end_idx; ++i) {
            data[i] = dist(engine);
        }
    }
}
*/
template <typename T>
void CMatrix<T>::fill(int start_idx, int end_idx) {
    if constexpr (std::is_same_v<int, T>) {
        std::mt19937 engine(std::random_device{}());
        std::uniform_int_distribution<int> dist(1, 10);
        auto generate_random_number = [&]() { return dist(engine); };
        // Llenar solo la parte correspondiente de 'start_idx' a 'end_idx'
        std::generate(data.begin() + start_idx, data.begin() + end_idx, generate_random_number);
    } 
    else if constexpr (std::is_same_v<double, T>) {
        std::mt19937 engine(std::random_device{}());
        std::uniform_real_distribution<double> dist(1.0, 10.0);
        auto generate_random_number = [&]() { return dist(engine); };
        std::generate(data.begin() + start_idx, data.begin() + end_idx, generate_random_number);
    } 
    else if constexpr (std::is_same_v<float, T>) {
        std::mt19937 engine(std::random_device{}());
        std::uniform_real_distribution<double> dist(1.0, 10.0);
        auto generate_random_number = [&]() { return dist(engine); };
        std::generate(data.begin() + start_idx, data.begin() + end_idx, generate_random_number);
    }
}

/////////////////////////////// end fill with random and Template specialization.
// Sobrecarga del operador << para imprimir la matriz
template <typename U>
std::ostream &operator<<(std::ostream &os, const CMatrix<U> &p) {
    for (size_t i = 0; i < p.size; i++) {
        for (size_t j = 0; j < p.size; j++) {
            os << p.data[p.size * i + j] << " ";
        }
        os << std::endl;
    }
    return os;
}
//////////////

template <typename U>
CMatrix<U> operator*(const CMatrix<U> &A, const CMatrix<U> &B) {
  CMatrix<U> prod(A.size);
  for (size_t i = 0; i < prod.size; i++) {
    for (size_t j = 0; j < prod.size; j++) {
      prod.data[prod.size * i + j] = 0;
      for (size_t k = 0; k < prod.size; k++) {
        prod.data[prod.size * i + j] +=
            A.data[prod.size * i + k] * B.data[prod.size * k + j];
      }
    }
  }
  return prod;
}
template <typename U>
CMatrix<U> operator+(const CMatrix<U> &A, const CMatrix<U> &B) {
  CMatrix<U> sum(A.size);
  for (size_t i = 0; i < sum.size; i++) {
    for (size_t j = 0; j < sum.size; j++) {
      sum.data[sum.size * i + j] =
          A.data[sum.size * i + j] + B.data[sum.size * i + j];
    }
  }
  return sum;
}

