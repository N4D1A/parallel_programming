# Assignment 3 - INF3201
## npa013 & jho046
### Running the sequential solutions
In order to run the sequential solutions, you must compile it. Navigate to the code/FrostTrap_OpenMP folder and run "make FrostTrap". The resulting program is named "FrostTrap" and can be run as such:   
./FrostTrap <simple/dbuf/rb> [dump]

### Running the OpenMP solutions
In order to run the OpenMP solutions, similarly to the sequential solution it must first be compiled. This can be done by navigating into the same folder code/FrostTrap_OpenMP and running "make FrostTrapOmp". The resulting program is named "FrostTrapOmp" and has the same usage as the seqtuential one:  
./FrostTrapOmp <simple/dbuf/rb> [dump]

### Running the CUDA solutions
The CUDA soltution does not need to be compiled before running, as it uses a Python wrapper script to run the CUDA C code. In order to run it, first navigate into the code/FrostTrap_Cuda folder, and then run the following command:
python3 FrostTrap.py <simple/dbuf/rb> [-W <width> -H <height> -R resultfile.json -v -g -p]  
The FrostTrap.py script supports more arguments than those in the example above. See the script itself for examples. 
