# System dependent definitions

FC=	nvc
FFLAGS=	-O3 -g  
LFLAGS= 

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
