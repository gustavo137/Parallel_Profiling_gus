CXX=nvc++
CXXFLAGS=-fast -acc=gpu -gpu=pinned -Minfo=all

exe: mandelbrot.o main.o
	$(CXX) $(CXXFLAGS) -o $@ $^

.PHONY: clean
clean:
	rm -f *.o core *.lst *.ptx *.pgprof *.cubin *.s *.x *.mod *.nvprof
