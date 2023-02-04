#PROFILER = scorep
#MPICC = $(PROFILER) mpicc
MPICC = mpicc
CC = cc
#CCP = mpicc-vt
CFLAGS = -O2 -Wall
LDFLAGS = -lpthread -lm
TARGS = RoadMap RoadMapGProf

all: $(TARGS) static dynamic

static: RoadMapStatic.c
	$(MPICC) $(CFLAGS) $< -o RoadMapStatic $(LDFLAGS)

dynamic: RoadMapDynamic.c
	$(MPICC) $(CFLAGS) $< -o RoadMapDynamic $(LDFLAGS)

RoadMap: RoadMap.c complex_lib.h
	$(CC) $(CFLAGS) $< -o $@ $(LDFLAGS)

RoadMapGProf: RoadMap.c complex_lib.h
	$(CC) $(CFLAGS) -pg $< -o $@ $(LDFLAGS)

clean:
	rm -f $(TARGS) RoadMapStatic RoadMapDynamic result* gmon.* *~

cleandata:
	rm data/*.data
