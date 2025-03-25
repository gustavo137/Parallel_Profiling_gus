#ifndef MTL_H
#define MTL_H

#ifdef _OPENMP
#include <omp.h>
#endif

#include <vector>
#include <stdexcept>
#include <concepts>

namespace mtl {

  template<std::floating_point T>
  class Matrix {

  public:
    using value_type = T;
    using size_type = std::size_t;
    using reference = T&;
    using const_reference = const T&;
    constexpr Matrix(std::size_t r, std::size_t c) : num_rows_{r}, num_cols_{c}, data_(num_rows_ * num_cols_) {} 
    constexpr Matrix(std::size_t r, std::size_t c, const T& v) : num_rows_{r}, num_cols_{c},  data_(num_rows_ * num_cols_, v) {} 
    constexpr Matrix(const Matrix&) = default;
    constexpr Matrix(Matrix &&) = default;
    constexpr Matrix& operator=(const Matrix &) = default;
    constexpr Matrix& operator=(Matrix &&) = default;
    constexpr ~Matrix() = default;
  
    constexpr inline reference operator() (std::size_t j, std::size_t i) noexcept {return data_[j * num_cols_ + i];}
    constexpr inline const_reference operator() (std::size_t j, std::size_t i) const noexcept {return data_[j * num_cols_ + i];}
    constexpr inline size_type num_rows() const noexcept {return num_rows_;}
    constexpr inline size_type num_cols() const noexcept {return num_cols_;}
    
    constexpr inline auto tol() const noexcept { 
      if constexpr(std::is_same_v<T,float>) {
        constexpr float tol = 1e-2; return tol;
      } else { 
      constexpr double tol = 1e-5; return tol;
      }
    }

  private:

    size_type num_rows_;
    size_type num_cols_;
    std::vector<value_type> data_;
  };

  template<std::floating_point T>
  constexpr inline auto check_mulcompatible_size(const Matrix<T>& lhs, const Matrix<T>& rhs) {
    if (lhs.num_cols() == rhs.num_rows()) return true;
    return false;
  }

  template<std::floating_point T>
  constexpr inline bool check_equal_size(const Matrix<T>& lhs, const Matrix<T>& rhs) {
    if (lhs.num_rows() == rhs.num_rows() && lhs.num_cols() == rhs.num_cols()) return true;
    return false;
  }

  template<std::floating_point T>
  constexpr bool operator==(const Matrix<T>& lhs, const Matrix<T>& rhs) {  
    if (!check_equal_size(lhs, rhs)) return false;
    std::size_t errs{0};
    #pragma omp parallel for reduction(+:errs) collapse(2) 
    for (std::size_t ii = 0; ii < lhs.num_rows(); ++ii) {
      for (std::size_t jj = 0; jj < lhs.num_cols(); ++jj) {
        if ( std::abs( lhs(ii, jj) - rhs(ii, jj) ) > std::max(lhs.tol(), rhs.tol()) ) ++errs;
      }
    }
    if (errs > 0) return false;
    return true;
  }

  template<std::floating_point T>
  constexpr auto mmp_parallel_tiled(const Matrix<T>& A, const Matrix<T>& B) {
    if (!check_mulcompatible_size(A,B)) throw std::length_error{"Incompatible length matrix-matrix product"};
    const auto Ar_blocksize = std::min(static_cast<int>(A.num_rows()), 32);
    const auto Ac_blocksize = std::min(static_cast<int>(A.num_cols()), 32);
    const auto Bc_blocksize = std::min(static_cast<int>(B.num_cols()), 32);

    Matrix<T> C{A.num_rows(), B.num_cols(), 0.0};
    #pragma omp parallel for collapse(2) 
    for (std::size_t ii=0; ii < A.num_rows(); ii += Ar_blocksize) {
      for (std::size_t jj=0; jj < B.num_cols(); jj += Bc_blocksize) {
        for (std::size_t kk = 0; kk < A.num_cols(); kk += Ac_blocksize) {
          for (std::size_t i=ii; i < std::min(ii+Ar_blocksize, A.num_rows()); ++i) {
            for (std::size_t j=jj; j < std::min(jj+Bc_blocksize, B.num_cols()); ++j) {
              for (std::size_t k=kk; k <  std::min(kk+Ac_blocksize, A.num_cols()) ; ++k) {
                C(i, j) += A(i, k) * B(k, j);
              }
            }
          }
        }
      }
    }
    return C;
  }

  template<std::floating_point T>
  constexpr auto mmp_parallel_blocks(const Matrix<T>& A, const Matrix<T>& B) {
    if (!check_mulcompatible_size(A,B)) throw std::length_error{"Incompatible length matrix-matrix product"};

    Matrix<T> D{B.num_cols(), B.num_rows()};
    #pragma omp parallel for collapse(2) 
    for (std::size_t ii=0; ii < D.num_rows(); ++ii) {
      for (std::size_t jj=0; jj < D.num_cols(); ++jj) {
        D(ii,jj) = B(jj,ii);
      }
    }
    Matrix<T> C{A.num_rows(), B.num_cols(), 0.0};
    const auto Ar_blocksize = std::min(static_cast<int>(A.num_rows()), 32);
    const auto Bc_blocksize = std::min(static_cast<int>(B.num_cols()), 32);
    #pragma omp parallel for collapse(2) 
    for (std::size_t ii=0; ii < A.num_rows(); ii += Ar_blocksize) {
      for (std::size_t jj=0; jj < B.num_cols(); jj += Bc_blocksize) {
        for (std::size_t i=ii; i < std::min(ii+Ar_blocksize, A.num_rows()); ++i) {
          for (std::size_t j=jj; j < std::min(jj+Bc_blocksize, B.num_cols()); ++j) {
            T sum{0.0};
            #pragma omp simd reduction(+:sum)
            for (std::size_t k = 0; k < A.num_cols(); ++k) {
              sum += A(i, k) * D(j,k);
            }
            C(i, j) = sum;
          }
        }
      }
    }
    return C;
  }

  template<std::floating_point T>
  constexpr auto mmp_parallel(const Matrix<T>& A, const Matrix<T>& B) {
    if (!check_mulcompatible_size(A,B)) throw std::length_error{"Incompatible length matrix-matrix product"};
    Matrix<T> C{A.num_rows(), B.num_cols(), 0.0};
    #pragma omp parallel for collapse(2) 
    for (std::size_t i = 0; i < A.num_rows(); ++i) {
      for (std::size_t j = 0; j < B.num_cols(); ++j) {
        T sum{0};
        #pragma omp simd reduction(+:sum)
        for (std::size_t k = 0; k < A.num_cols(); ++k) {
          sum += A(i, k) * B(k, j);
        }
	C(i, j) = sum;
      }
    }
    return C;
  }

  template<std::floating_point T>
  constexpr auto mmp_serial(const Matrix<T>& A, const Matrix<T>& B) {
    if (!check_mulcompatible_size(A,B)) throw std::length_error{"Incompatible length matrix-matrix product"};
    Matrix<T> C{A.num_rows(), B.num_cols(), 0.0};
    for (std::size_t i = 0; i < A.num_rows(); ++i) {
      for (std::size_t j = 0; j < B.num_cols(); ++j) {	      
        for (std::size_t k = 0; k < A.num_cols(); ++k) {
          C(i, j) += A(i, k) * B(k, j);
        }
      }
    }
    return C;
  }
}

#endif

