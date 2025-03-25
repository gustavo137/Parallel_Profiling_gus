# System dependent definitions

FC=	gfortran 
FFLAGS=  -O2
LFLAGS= 

# System independent definitions

MF=	Makefile

EXE=	./exe

SRC= \
	boundary.f90 \
	utilities.f90 \
	cfd.f90 \
	cfdio.f90 \
	jacobi.f90

#
# No need to edit below this line
#

.SUFFIXES:
.SUFFIXES: .f90 .o

OBJ=	$(SRC:.f90=.o)

.f90.o:
	$(FC) $(FFLAGS) -c $<

all:	$(EXE)

$(EXE):	$(OBJ)
	$(FC) $(FFLAGS) -o $@ $(OBJ) $(LFLAGS)

$(OBJ):	$(MF)

utilities.o : cfdio.o
cfd.o:	boundary.o jacobi.o cfdio.o 

tar:
	tar cvf cfd.tar $(MF) $(INC) $(SRC)

clean:
	rm -f $(OBJ) *.mod velocity.dat colourmap.dat cfd.plt core
