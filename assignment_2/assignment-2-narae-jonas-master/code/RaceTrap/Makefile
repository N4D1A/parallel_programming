CC = gcc

CFLAGS = -O2 -Wall

LIB = -lm

all: StopWatch.o RaceTrap RaceTrapHybrid RaceTrapLB

RaceTrap: RaceTrap.c
	$(CC) $(CFLAGS) RaceTrap.c StopWatch.o -o RaceTrap $(LIB)

RaceTrapHybrid: RaceTrapHybrid.c
	$(CC) $(CFLAGS) -fopenmp RaceTrapPar.c StopWatch.o -o RaceTrapHybrid $(LIB)
	
RaceTrapLB: RaceTrapLB.c
	$(CC) $(CFLAGS) -fopenmp RaceTrapLB.c StopWatch.o -o RaceTrapLB $(LIB)

StopWatch.o: StopWatch.c
	$(CC) -c StopWatch.c

clean:
	rm -f *~ *.o core* RaceTrap RaceTrapHybrid RaceTrapLB

cleandata:
	rm -f data/*
