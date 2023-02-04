#!/bin/sh

if [ $# -lt 3 ]; then
    echo "Usage: static|dynamic|staticRR|sequential num_procs num_hosts"
    exit
fi

if [ "$1" = "static" ]; then
    # User wants to test static implementation, compile the executable
    make static
    # Make folder for results
    mkdir results-stat
    # Make list of hosts to use, only once
    # !! When comparing results, make hostfile in advance and keep the same hosts to minimize the effect of hardware changes
    #sh generate_hosts.sh $3
    
    # Now run the executable 5 times (iterations)
    for i in $(eval echo {1..5}); do
        # Inform the user of progress
        echo "Static, procs $2, nodes $3, iter $i"
        
        # !!! Uncomment if using Score-P for profiling !!!
        #mpirun -np $2 -hostfile hostfile -x SCOREP_TIMER="clock_gettime" -x SCOREP_ENABLE_TRACING=1 -x SCOREP_EXPERIMENT_DIRECTORY="profile-stat" -x SCOREP_FILTERING_FILE=scorep.filter -x SCOREP_TOTAL_MEMORY=1024M RoadMapStatic
        #mv profile-stat "results-stat/nodes_$3-threads_$2-#$i"
        # !!! Uncomment if using Score-P for profiling !!!

        # Run the executable
        mpirun -np $2 -hostfile hostfile RoadMapStatic x rows
        # Make a sub-folder for this iteration
        mkdir "results-stat/nodes_$3-threads_$2-#$i"
        # Put the result txt file there
        cp result-stat.txt "results-stat/nodes_$3-threads_$2-#$i/result.txt"
    done

    rm result-stat.txt

elif [ "$1" = "staticRR" ]; then
    # User wants to test static implementation, compile the executable
    make static
    # Make folder for results
    mkdir results-statRR
    # Make list of hosts to use, only once
    # !! When comparing results, make hostfile in advance and keep the same hosts to minimize the effect of hardware changes
    #sh generate_hosts.sh $3

    # Now run the executable 5 times (iterations)
    for i in $(eval echo {1..5}); do
        # Inform the user of progress
        echo "StaticRR, procs $2, nodes $3, iter $i"
        
        # !!! Uncomment if using Score-P for profiling !!!
        #mpirun -np $2 -hostfile hostfile -x SCOREP_TIMER="clock_gettime" -x SCOREP_ENABLE_TRACING=1 -x SCOREP_EXPERIMENT_DIRECTORY="profile-statRR" -x SCOREP_FILTERING_FILE=scorep.filter -x SCOREP_TOTAL_MEMORY=1024M RoadMapStatic
        #mv profile-stat "results-statRR/nodes_$3-threads_$2-#$i"
        # !!! Uncomment if using Score-P for profiling !!!

        # Run the executable
        mpirun -np $2 -hostfile hostfile RoadMapStatic x rowsrr
        # Make a sub-folder for this iteration
        mkdir "results-statRR/nodes_$3-threads_$2-#$i"
        # Put the result txt file there
        cp result-statRR.txt "results-statRR/nodes_$3-threads_$2-#$i/result.txt"
    done

    rm result-statRR.txt

elif [ "$1" = "dynamic" ]; then
    # User wants to test dynamic implementation, compile the executable
    make dynamic
    # Make folder for results
    mkdir results-dyna
    # Make list of hosts to use, only once
    # !! When comparing results, make hostfile in advance and keep the same hosts to minimize the effect of hardware changes
    #sh generate_hosts.sh $3

    # Now run the executable 5 times (iterations)
    for i in $(eval echo {1..5}); do
        # Inform the user of progress
        echo "Dynamic, procs $2, nodes $3, iter $i"
        # Uncomment if using Score-P for profiling 
        #mpirun -np $2 -hostfile hostfile -x SCOREP_TIMER="clock_gettime" -x SCOREP_ENABLE_TRACING=1 -x SCOREP_EXPERIMENT_DIRECTORY="profile-dyna" -x SCOREP_FILTERING_FILE=scorep.filter -x SCOREP_TOTAL_MEMORY=1024M RoadMapStatic
        #mv profile-dyna "results-dyna/nodes_$3-threads_$2-#$i"
        
        # Run the executable
        mpirun -np $2 -hostfile hostfile RoadMapDynamic
        # Make a sub-folder for this iteration
        mkdir "results-dyna/nodes_$3-threads_$2-#$i"
        # Put the result txt file there
        cp result-dyna.txt "results-dyna/nodes_$3-threads_$2-#$i/result.txt"
    done

    rm result-dyna.txt

elif [ "$1" = "sequential" ]; then
    make RoadMapGProf
    # Not done!
fi
