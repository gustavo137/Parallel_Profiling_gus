MODULE utilities

      implicit none

      contains

      subroutine copy_back(new, old, m, n)

      integer :: m, n
      double precision, dimension(0:m+1, 0:n+1) :: old, new

      integer :: i, j

      do j=1,n
        do i=1,m
          old(i,j) = new(i,j)
        end do
      end do
   
     end subroutine copy_back

     subroutine finalize(psi, m, n, scalefactor)
        use cfdio
        implicit none
        
        integer :: m, n, scalefactor
        double precision, dimension(0:m+1, 0:n+1) :: psi

        !  Output results
        call writedatafiles(psi, m, n, scalefactor)
        !  Output gnuplot file
        call writeplotfile(m, n, scalefactor)


     end subroutine finalize

END MODULE utilities
