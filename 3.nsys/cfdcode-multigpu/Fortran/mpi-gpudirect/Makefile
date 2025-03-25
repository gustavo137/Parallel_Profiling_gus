# Set the target system
# Can use one of this list: archer, cirrus or linux

NVHPC_CUDA_HOME=/cineca/prod/opt/compilers/nvhpc/2022/binary/Linux_x86_64/2022/cuda/

# System dependent definitions

FC=	mpif90 -acc -gpu=cc80 -g
FFLAGS=	-fast
LFLAGS=

# System independent definitions

MF=	Makefile

EXE=	cfd

SRC= \
	boundary.f90 \
	cfd.f90 \
	cfdio.f90 \
	jacobi.f90

#
# No need to edit below this line except for code-specific module
# dependencies right at the bottom
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

tar:
	tar cvf cfd.tar $(MF) $(INC) $(SRC)

clean:
	rm -f $(OBJ) $(EXE) *.mod velocity.dat colourmap.dat cfd.plt core

# Module dependencies

cfd.o:	boundary.o jacobi.o cfdio.o
