module jacobi

  implicit none

contains

subroutine jacobistep(psinew, psi, m, n)

  integer :: m, n, i, j

  double precision, dimension(0:m+1, 0:n+1) :: psinew, psi

  !$acc parallel loop collapse(2)
  do j = 1, n
      do i = 1, m
          psinew(i, j) = 0.25d0*(psi(i+1, j) + psi(i-1, j) + &
                             psi(i, j+1) + psi(i, j-1)     )
      end do
  end do
  !$acc end parallel loop


end subroutine jacobistep


double precision function deltasq(new, old, m, n)

  integer :: m, n, i, j
  double precision, dimension(0:m+1, 0:n+1) :: new, old
  double precision :: deltasq

  integer :: ierr

  deltasq = 0.d0
  !$acc parallel loop collapse(2) reduction(+:deltasq) present(new,old)
  do j = 1, n
    do i = 1, m
      deltasq = deltasq + (new(i,j)-old(i,j))**2
    end do
  end do
  !$acc end parallel loop

end function deltasq

end module jacobi
                                    


