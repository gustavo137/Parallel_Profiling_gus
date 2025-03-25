module boundary

  implicit none

contains

subroutine boundarypsi(psi, m, n, b, h, w)

  integer :: m, n, b, h, w  
  double precision, dimension(0:m+1, 0:n+1) :: psi
  integer :: i, j

!  Set the boundary conditions on the bottom edge

  do i = b+1, b+w-1
     psi(i, 0) = float(i-b)
  end do

  do i = b+w, m
     psi(i, 0) = float(w)
  end do

  !  Set the boundary conditions on the right hand side

  do j = 1, h

     psi(m+1,j) = float(w)
     
  end do

  do j = h+1, h+w-1

     psi(m+1,j) = float(w-j+h)

  end do

end subroutine boundarypsi

end module boundary
