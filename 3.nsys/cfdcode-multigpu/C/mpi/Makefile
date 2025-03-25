# Set the target system
# Can use one of this list: archer, cirrus or linux

NVHPC_CUDA_HOME=/cineca/prod/opt/compilers/nvhpc/2022/binary/Linux_x86_64/2022/cuda/

CC=	mpicc
CFLAGS=	-O3
LFLAGS=	-lm

# System independent definitions

MF=	Makefile

EXE=	cfd

INC= \
	arraymalloc.h \
	boundary.h \
	cfdio.h \
	jacobi.h

SRC= \
	arraymalloc.c \
	boundary.c \
	cfd.c \
	cfdio.c \
	jacobi.c

OUT= \
	velocity.dat \
	colourmap.dat \
	cfd.plt

#
# No need to edit below this line
#

.SUFFIXES:
.SUFFIXES: .c .o

OBJ=	$(SRC:.c=.o)

.c.o:
	$(CC) $(CFLAGS) -c $<

all:	$(EXE)

$(OBJ):	$(INC)

$(EXE):	$(OBJ)
	$(CC) $(CFLAGS) -o $@ $(OBJ) $(LFLAGS)

$(OBJ):	$(MF)

tar:
	tar cvf cfd.tar $(MF) $(INC) $(SRC)

clean:
	rm -f $(OBJ) $(EXE) $(OUT) core *~
