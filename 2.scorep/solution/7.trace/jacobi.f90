module jacobi

  implicit none

contains

subroutine jacobistep(psinew, psi, m, n)

  integer :: m, n
  double precision, dimension(0:m+1, 0:n+1) :: psinew, psi

  psinew(1:m, 1:n) = 0.25d0*(psi(2:m+1, 1:n) + psi(0:m-1, 1:n) + &
                             psi(1:m, 2:n+1) + psi(1:m, 0:n-1)     )

end subroutine jacobistep


double precision function deltasq(new, old, m, n)

  integer :: m, n
  double precision, dimension(0:m+1, 0:n+1) :: new, old

  integer :: ierr

  deltasq =   sum((new(1:m,1:n)-old(1:m,1:n))**2)

end function deltasq

end module jacobi
                                    


