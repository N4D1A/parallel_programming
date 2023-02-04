#!/bin/bash -l

if [ $# -lt 2 ]
  then
    echo "Usage: num_procs num_hosts"
    exit
fi

sh generate_hosts.sh $2
mpirun -np $1 -hostfile hostfile mpi-hello


