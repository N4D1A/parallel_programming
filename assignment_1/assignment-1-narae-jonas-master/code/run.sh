#!/bin/bash -l

if [ $# -lt 3 ]
  then
    echo "Usage: static|dynamic num_procs num_hosts"
    exit
fi

sh generate_hosts.sh $3

# You need to create these files (RoadMapStatic & RoadMapDynamic) first - Makefile!
if [ "$1" = "static" ]; then
  make clean
  make static
  mpirun -np $2 -hostfile hostfile -x SCOREP_ENABLE_TRACING=1 -x SCOREP_EXPERIMENT_DIRECTORY=profile -x SCOREP_FILTERING_FILE=scorep.filter -x SCOREP_TOTAL_MEMORY=1024M RoadMapStatic
elif [ "$1" = "dynamic" ]; then
  make clean
  make dynamic
  mpirun -np $2 -hostfile hostfile -x SCOREP_ENABLE_TRACING=1 -x SCOREP_EXPERIMENT_DIRECTORY=profile -x SCOREP_FILTERING_FILE=scorep.filter -x SCOREP_TOTAL_MEMORY=1024M RoadMapDynamic
fi

