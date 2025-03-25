# System dependent definitions

FC=	nvc
FFLAGS=	-O3 -g -I /g100_work/PROJECTS/spack/preprod/01/install/0.18/linux-centos8-skylake_avx512/gcc-8.4.1/nvhpc-22.3-jgtetvzbyjbc2o6qpmlcjcjxkp5vi7r5/Linux_x86_64/22.3/cuda/include/nvtx3/  
LFLAGS= -cuda -lnvToolsExt 

# System independent definitions

MF=	Makefile

EXE=	exe

SRC= \
	boundary.c \
	cfd.c \
	cfdio.c \
	jacobi.c \
	arraymalloc.c

#
# No need to edit below this line
#

.SUFFIXES:
.SUFFIXES: .c .o

OBJ=	$(SRC:.c=.o)

.c.o:
	$(FC) $(FFLAGS) -c $<

all:	$(EXE)

$(EXE):	$(OBJ)
	$(FC) $(FFLAGS) -o $@ $(OBJ) $(LFLAGS)

$(OBJ):	$(MF)

cfd.o:	boundary.o jacobi.o cfdio.o arraymalloc.o

tar:
	tar cvf cfd.tar $(MF) $(INC) $(SRC)

clean:
	rm -f $(OBJ) $(EXE) *.mod velocity.dat colourmap.dat cfd.plt core
