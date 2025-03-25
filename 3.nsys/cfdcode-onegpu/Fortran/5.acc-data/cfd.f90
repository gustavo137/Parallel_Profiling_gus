!###################################################################!
!                                                                   !
! MODIFIED FROM https://github.com/davidhenty/cfd/tree/master/F-OMP !
!                                                                   !
!###################################################################!
program cfd

  use boundary
  use jacobi
  use cfdio
  use nvtx

  implicit none

! Output frequency
  
  integer, parameter :: printfreq = 1000
  double precision :: error, bnorm
  double precision, parameter :: tolerance = 1.0d-4

! Main arrays

  double precision, allocatable ::  psi(:,:), psitmp(:,:)

! Command-line arguments

  integer :: scalefactor,  numiter
  double precision :: re  ! re = 3.7 seems to be stability limit with Jacobi
  integer, parameter :: maxline = 32
  character(len=maxline) :: tmparg

!  Basic sizes of simulation

  integer, parameter :: bbase = 10
  integer, parameter :: hbase = 15
  integer, parameter :: wbase =  5
  integer, parameter :: mbase = 32
  integer, parameter :: nbase = 32

  logical :: checkerr = .false.

!  Some auxiliary parameters and variables

  integer :: m, n, b, h, w, i,j
  integer :: iter

  double precision :: tstart, tstop, ttot, titer, modvsq, hue

!  Are we stopping based on tolerance?

   call nvtxStartRange("SimpleCFD")

  if (tolerance .gt. 0.0) checkerr = .true.

!  Read in parameters

  if (command_argument_count() /= 2 .and. command_argument_count() /= 3) then

     write(*,*) 'Usage: cfd <scale> <numiter> [reynolds]'
     stop

  end if

  call get_command_argument(1, tmparg)
  read(tmparg,*) scalefactor
  call get_command_argument(2, tmparg)
  read(tmparg,*) numiter
  re = -1.0
     
  if (.not. checkerr) then
     write(*,fmt='('' Scale factor = '',i3,'', iterations = '', i6)') &
           scalefactor, numiter
  else
     write(*,fmt='('' Scale factor = '',i3,'', iterations = '', i6, &
          &'', tolerance = '', g11.4)') scalefactor, numiter, tolerance
  end if


!  Calculate b, h & w and m & n
        
  b = bbase*scalefactor 
  h = hbase*scalefactor
  w = wbase*scalefactor 
  m = mbase*scalefactor
  n = nbase*scalefactor

  re = re / dble(scalefactor)

  write(*,fmt='('' Running CFD on '', i8, '' x '', i8, &
       &'' grid in serial '')') m, n

! Allocate arrays, including halos on psi and tmp

  allocate(psi(0:m+1, 0:n+1))
  allocate(psitmp(0:m+1, 0:n+1))

! Zero the psi array
  psi(:,:) = 0.0

! Set the psi boundary condtions which are constant
  call nvtxStartRange("boundary",1) 
  call boundarypsi(psi, m, n, b, h, w)
  call nvtxEndRange()

! Compute normalisation factor for error

  bnorm = sum(psi(:,:)**2)
  bnorm = sqrt(bnorm)

! Begin iterative Jacobi loop

  write(*,*)
  write(*,*) 'Starting main loop ...'
  write(*,*)

  tstart = gettime()

  call nvtxStartRange("main_loop",2)

  !$acc enter data copyin(psi(0:m+1, 0:n+1)) create(psitmp(0:m+1, 0:n+1))
  do iter = 1, numiter

     call nvtxStartRange("jacobi",5)
     call jacobistep_acc(psitmp, psi, m, n)
     call nvtxEndRange()

!  Compute current error value if required
 
     if (checkerr .or. iter == numiter) then
        error = deltasq(psitmp, psi, m, n)
        error = sqrt(error)
        error = error / bnorm
     end if

!  Quit early if we have reached required tolerance

     if (checkerr) then
        if (error .lt. tolerance) then
           write(*,*) 'CONVERGED iteration ', iter, ': terminating'
           exit
        end if
     end if

!  Copy back
     call nvtxStartRange("copy_back",6)
     !$acc parallel loop collapse(2) present(psi,psitmp)
     do j=1,m
        do i=1,n
          psi(i,j) = psitmp(i,j)
        end do
     end do     
     !$acc end parallel loop
     call nvtxEndRange()

!  End iterative Jacobi loop

     if (mod(iter,printfreq) == 0) then

        if (.not. checkerr) then
           write(*,*) 'completed iteration ', iter
        else
           write(*,*) 'completed iteration ', iter, ', error = ', error
        end if

     end if

  end do
  !$acc exit data delete(psitmp) copyout(psi)

  call nvtxEndRange()

  if (iter .gt. numiter) iter = numiter

  tstop = gettime()

  ttot  = tstop-tstart
  titer = ttot/dble(iter)

  write(*,*) 
  write(*,*) '... finished'
  write(*,*)
  write(*,fmt='('' After    '', i6, '' iterations, error is '', g11.4)') &
        iter, error
  write(*,fmt='('' Time for '', i6, '' iterations was '',&
        &g11.4, '' seconds'')') iter, ttot
  write(*,fmt='('' Each individual iteration took '', g11.4, '' seconds'')') &
        titer
  write(*,*)
  write(*,*) 'Writing output file ...'

  call nvtxStartRange("write_out",3)
! Output results
  call writedatafiles(psi, m, n, scalefactor)
! Output gnuplot file
  call writeplotfile(m, n, scalefactor)
  call nvtxEndRange()

! Finish

  write(*,*) ' ... finished'
  write(*,*)
  write(*,*) 'CFD completed'
  write(*,*)
  call nvtxEndRange()
end program cfd

