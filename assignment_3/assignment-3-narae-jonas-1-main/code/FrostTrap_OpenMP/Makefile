CC = gcc

# Use gnu99 instead of c99 to avoid warnings about gethostname.
CFLAGS = -std=gnu99 -O2 -Wall  #-pg -lprofiler

all: FrostTrap FrostTrapOmp

FrostTrap: FrostTrap.c
	$(CC) $(CFLAGS) -fopenmp -Wall FrostTrap.c -o FrostTrap

FrostTrapOmp: FrostTrapOmp.c
	$(CC) $(CFLAGS) -fopenmp -Wall FrostTrapOmp.c -o FrostTrapOmp

clean:
	@rm -fv *.o *~ core* FrostTrap FrostTrapOmp

cleandata:
	@rm -fv data/*.data
