TARGS = mpi-hello


all: $(TARGS)

clean: 
	rm $(TARGS) 

% : %.c
	mpicc -o $@ $<

