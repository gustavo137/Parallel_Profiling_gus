program cfd

  use mpi

  use boundary
  use jacobi
  use cfdio

  implicit none

! Output frequency
  
  integer, parameter :: printfreq = 1

! Variables associated with convergence

  double precision :: localerror, error, localbnorm, bnorm

! Set tolerance for convergence; zero or negative means do not check

  double precision, parameter :: tolerance = 0.0

! Main arrays

  double precision, allocatable ::  psi(:,:), psitmp(:,:)

! Command-line arguments

  integer :: scalefactor,  numiter, i, j

  double precision :: re  ! re = 3.7 seems to be stability limit with Jacobi

  integer, parameter :: maxline = 32
  character(len=maxline) :: tmparg

!  Basic sizes of simulation

  integer, parameter :: bbase = 10
  integer, parameter :: hbase = 15
  integer, parameter :: wbase =  5
  integer, parameter :: mbase = 1024
  integer, parameter :: nbase = 32

  logical :: irrotational = .true., checkerr = .false.

!  Some auxiliary parameters and variables

  integer :: m, n, ln, b, h, w
  integer :: iter

  double precision :: tstart, tstop, ttot, titer, modvsq, hue

!  Variables needed for parallelisation

  integer :: comm, rank, size, ierr

!  Variables needed for gpu binding

  integer :: ndev

!  Are we stopping based on tolerance?

  if (tolerance .gt. 0.0) checkerr = .true.

!  Parallel initialisation

  comm = MPI_COMM_WORLD

  call mpi_init(ierr)

  call mpi_comm_rank(comm, rank, ierr)
  call mpi_comm_size(comm, size, ierr)

!  Read in parameters

  if (command_argument_count() /= 2) then

     if (rank == 0) write(*,*) 'Usage: cfd <scale> <numiter>'
     call mpi_finalize(ierr)
     stop

  end if

  if (rank == 0) then 

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
        
   write(*,*) 'Irrotational flow'

   end if

!  Broadcast runtime parameters to all the other processors
  
  call mpi_bcast(scalefactor,  1, MPI_INTEGER, 0, comm, ierr)
  call mpi_bcast(numiter,      1, MPI_INTEGER, 0, comm, ierr)
  call mpi_bcast(re, 1,  MPI_DOUBLE_PRECISION, 0, comm, ierr)
  call mpi_bcast(irrotational, 1, MPI_LOGICAL, 0, comm, ierr)

  write(*,fmt='('' Scale factor = '',i3,'', iterations = '', i6)') &
             scalefactor, numiter

!  Calculate b, h & w and m & n
        
  b = bbase*scalefactor 
  h = hbase*scalefactor
  w = wbase*scalefactor 
  m = mbase*scalefactor
  n = nbase*scalefactor

  re = re / dble(scalefactor)

!  Calculate local size

  ln = n/size

!  Consistency check

  if (size*ln /= n) then

     if (rank == 0) write(*,*) 'ERROR: n = ', n, ' does not divide onto ', &
          size, ' processes'

     call mpi_finalize(ierr)
     stop

  end if

  if (rank == 0) then
     write(*,fmt='('' Running CFD on '', i4, '' x '', i4, '' grid using '', &
                  &i4, '' process(es)'')') m, n, size
  end if

!  Allocate arrays, including halos on psi and tmp

  allocate(psi(0:m+1, 0:ln+1))
  allocate(psitmp(0:m+1, 0:ln+1))

!  Zero the psi array

  psi(:,:) = 0.0

!  Set the psi boundary condtions which are constant

   call boundarypsi(psi, m, ln, b, h, w, comm)

!  Compute normalisation factor for error

   localbnorm = sum(psi(:,:)**2)

!  Do a boundary swap of psi
  call haloswap(psi, m, ln, comm, rank, size)

  call mpi_allreduce(localbnorm, bnorm, 1, MPI_DOUBLE_PRECISION, &
                     MPI_SUM, comm, ierr)

   bnorm = sqrt(bnorm)

!  Begin iterative Jacobi loop

  if (rank == 0) then
     write(*,*)
     write(*,*) 'Starting main loop ...'
     write(*,*)
  end if
     
!  Barriers purely to get consistent timing - not needed for correctness

  call mpi_barrier(comm, ierr)

  tstart = mpi_wtime()

  do iter = 1, numiter

!  Compute the new psi based on the old one

!  Call function with no vorticity

        call jacobistep(psitmp, psi, m, ln)

!  Compute current error value if required
     
     if (checkerr .or. iter == numiter) then

        localerror = deltasq(psitmp, psi, m, ln)

        call mpi_allreduce(localerror, error, 1, MPI_DOUBLE_PRECISION, &
                           MPI_SUM, comm, ierr)

        error = sqrt(error)
        
        error = error / bnorm

     end if

!  Copy back

     DO j = 1, ln
       DO i = 1, m
         psi(i,j) = psitmp(i,j)
       END DO
     END DO

!  Do a boundary swap

     call haloswap(psi, m, ln, comm, rank, size)

!  Quit early if we have reached required tolerance

     if (checkerr) then
        if (error .lt. tolerance) then
           if (rank == 0) write(*,*) 'CONVERGED iteration ', iter, &
                                     ': terminating'
           exit
        end if
     end if

!  End iterative Jacobi loop

     if (mod(iter,printfreq) == 0) then

        if (rank == 0) then

           if (.not. checkerr) then
              write(*,*) 'completed iteration ', iter
           else
              write(*,*) 'completed iteration ', iter, ', error = ', error
           end if

        end if
     end if

  end do

  if (iter .gt. numiter) iter = numiter

  call mpi_barrier(comm, ierr)

  tstop = mpi_wtime()

  ttot  = tstop-tstart
  titer = ttot/dble(iter)

  if (rank == 0) then

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

  end if

! Finish

  call mpi_finalize(ierr)

  if (rank == 0) then
     write(*,*) ' ... finished'
     write(*,*)
     write(*,*) 'CFD completed'
     write(*,*)
  end if

end program cfd

