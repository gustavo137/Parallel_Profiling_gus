# System dependent definitions

CXX=	g++
CC=	gcc
CXXFLAGS= -Wall -Wextra -O3  -fopenmp -march=native -fstrict-aliasing -mprefer-vector-width=512 -ftree-vectorize -std=c++20 #-DUSE_FLOAT
CCFLAGS=  -Wall -Wextra -O3  -fopenmp -march=native -fstrict-aliasing -mprefer-vector-width=512 -ftree-vectorize            #-DUSE_FLOAT 

# System independent definitions

MF=	Makefile

EXE_CXX=	../bin/main_cpp.x
SRC_CXX=	main.cpp 
EXE_C=	../bin/main_c.x
SRC_C=	main.c 
EXE_S=	../bin/simple.x
SRC_S=	simple.c 
EXES=$(EXE_CXX) $(EXE_C)

all: $(EXES)
$(EXE_CXX):
	$(CXX) $(CXXFLAGS) $(SRC_CXX) -o $@

$(EXE_C):
	$(CC) $(CCFLAGS) $(SRC_C) -o $@

clean:
	rm -f ../bin/main_c.x ../bin/main_cpp.x 

