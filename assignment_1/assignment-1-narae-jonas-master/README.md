# assignment-1-narae-jonas
assignment-1-narae-jonas created by GitHub Classroom

### To compile:
  $ make

### To run static row block partition version:
  $ sh experiment.sh static {num_procs} {num_hosts}
 
### To run static row cyclic (round-robin) version (additional approach):
  $ sh experiment.sh staticRR {num_procs} {num_hosts}
  
### To run dynamic row partition (by each row) version :
  $ sh experiment.sh dynamic {num_procs} {num_hosts}
  
### To run dynamic row partition (by block) version :
  $ make; mpirun -np {num_procs} -hostfile hostfile RoadMapDynamic x {num_rows}

### To make mandelbrot image produced by each solution:
  $ make; mpirun -np {num_procs} -hostfile hostfile RoadMapStatic dump rows  
  $ make; mpirun -np {num_procs} -hostfile hostfile RoadMapStatic dump rowsRR  
  $ make; mpirun -np {num_procs} -hostfile hostfile RoadMapDynamic dump  
  

