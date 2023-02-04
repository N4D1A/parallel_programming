#include <stdio.h>
#include <math.h>
#include "complex_lib.h"
#include <sys/time.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <mpi.h>

#define WIDTH 2000
#define HEIGHT 2000

// The maximum number of iterations we will compute before giving up at a given coordinate
// This used to be 1024, but that masked out all the variation between 1-100, where most of the details are. 
#define MAX_ITERATIONS 100

int DO_DUMP = 0;    // true if we want to dump the iterations from the file
int ROWS = 0;       // true if we want to divide work by row blocks
int ROWSRR = 1;     // true if we want to divide work by row in round robin (default)
int zooms = 10;     // number of zooms before we stop
int crc = 0;        // used for debugging (compare parallel with sequential, for instance)

int roadMap[HEIGHT][WIDTH];
double box_x_min, box_x_max, box_y_min, box_y_max;

/** 
 * Dumping the roadMap array for later visualization. 
 */
void dump_data()
{
    char fname[256];
    FILE *fp;
    static int filenum = 0; 
    if (!DO_DUMP)
        return;
    
    int x, y;
    /* Stores the data as a Python datastructure for easy inspection and plotting */ 
    sprintf(fname, "data/roadmap-stat-out-%04d.data", filenum++);
    printf("Storing data to %s.\n", fname); 
    fp = fopen(fname, "w");
    fprintf(fp, "{\n"); 
    fprintf(fp, "  'expdata' : 'roadmap-stat',\n"); 
    fprintf(fp, "  'arr'     : [\n"); 
    for (y = 0; y < HEIGHT; y++) {
        fprintf(fp, "     [ "); 
        for (x = 0; x < WIDTH; x++) {
            fprintf(fp, "%d, ", roadMap[y][x]);
        }
        fprintf(fp, "],\n"); 
    }
    fprintf(fp, "], \n"); 
    fprintf(fp, "} \n");
    fclose(fp); 
}

/**
 * Translate from pixel coordinates to space coordinates
 * 
 * @param       x       Pixel coordinate
 * @returns     Space coordinate
 */
double translate_x(int x) {       
    return box_x_min + (((box_x_max-box_x_min)/WIDTH)*x);
}

/**
 * Translate from pixel coordinates to space coordinates
 * 
 * @param       y       Pixel coordinate
 * @returns     Space coordinate
 */
double translate_y(int y) {
    return box_y_min + (((box_y_max-box_y_min)/HEIGHT)*y);
}

/**
 * Mandelbrot divergence test
 * 
 * @param       x,y     Space coordinates
 * @returns     Number of iterations before convergance
 */
int solve(double x, double y)
{
    complex z = {0.0, 0.0};
    complex c = {x, y};
    int itt = 0;
    for (itt = 0; (itt < MAX_ITERATIONS) && (complex_magn2(z) <= 4.0); itt++) {
        z = complex_add(complex_squared(z), c); 
    }
    return itt;
}

void CreateMap_Rows(int my_rank, int comm_size) {
    int i, j; 
    int local_height = HEIGHT/comm_size; // height for each process(rank)
    int remainder = HEIGHT%comm_size; // remainder when the height is not evenly divided by the number of processes 

    if(my_rank!=0){
        if(my_rank==comm_size-1){ // if not evenly divided, let the last process do the rest
            int local_roadMap[local_height+remainder][WIDTH]; // array for calculated results for last process (and for sending them to whole result map)
            int k=0; // row count for local_roadMap array
            for(i=my_rank*local_height; i<((my_rank+1)*local_height)+remainder; i++){ 
                for(j=0; j<WIDTH; j++){
                    local_roadMap[k][j] = solve(translate_x(((i*WIDTH)+j)%WIDTH), translate_y(((i*WIDTH)+j)/WIDTH)); 
                }
                k++;
            }
            MPI_Send(local_roadMap, (local_height+remainder)*WIDTH, MPI_INT, 0, my_rank, MPI_COMM_WORLD); // send the calculated results to the master process
        }
        else{
            int local_roadMap[local_height][WIDTH]; // array for calculated results for each process (and for sending them to whole result map)
            int k=0; // row count for local_roadMap array
            for(i=my_rank*local_height; i<(my_rank+1)*local_height; i++){ 
                for(j=0; j<WIDTH; j++){
                    local_roadMap[k][j] = solve(translate_x(((i*WIDTH)+j)%WIDTH), translate_y(((i*WIDTH)+j)/WIDTH)); 
                }
                k++;
            }
            MPI_Send(local_roadMap, local_height*WIDTH, MPI_INT, 0, my_rank, MPI_COMM_WORLD); // send the calculated results to the master process
        }
    }
    
    if (my_rank==0){
        for(i=my_rank*local_height; i<(my_rank+1)*local_height; i++){ // master process calculates the first block (while waiting for the results of other processes)
            for(j=0; j<WIDTH; j++){
                roadMap[i][j] = solve(translate_x(((i*WIDTH)+j)%WIDTH), translate_y(((i*WIDTH)+j)/WIDTH));
            }
        }
        for(i=1; i<comm_size; i++){
            if(i==comm_size-1){ // receive the results from the last process
                MPI_Recv(roadMap[i*local_height], (local_height+remainder)*WIDTH, MPI_INT, i, i, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            }
            else{ // receive the results from each process
                MPI_Recv(roadMap[i*local_height], local_height*WIDTH, MPI_INT, i, i, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            }   
        }
        for(i=0;i<HEIGHT;i++){ 
            for(j=0;j<WIDTH;j++){
                crc += roadMap[i][j]; // get CRC from the combined roamMap
            }
        }
        dump_data();
    }
}

void CreateMap_RowsRR(int my_rank, int comm_size) {
    int i,j; 
    int interval = comm_size; // interval of processes in work
            
    if(my_rank!=0){
        int local_roadMap[HEIGHT/comm_size + 1][WIDTH]; // array for calculated results for each process. add one row in case the workload is not evenly divided 
        int k=0; // row count for local_roadMap array
        for (i=my_rank; i<HEIGHT; i+=interval){
            for(j=0; j<WIDTH; j++){
                local_roadMap[k][j] = solve(translate_x(((i*WIDTH)+j)%WIDTH), translate_y(((i*WIDTH)+j)/WIDTH)); 
            }
            k++;
        }
        MPI_Send(local_roadMap, (k+1)*WIDTH, MPI_INT, 0, k, MPI_COMM_WORLD);
        }

    else if(my_rank==0){

        for(i=my_rank; i<HEIGHT; i+=interval){ // master process calculates the first block (while waiting for the results of other processes)
            for(j=0; j<WIDTH; j++){
                roadMap[i][j] = solve(translate_x(((i*WIDTH)+j)%WIDTH), translate_y(((i*WIDTH)+j)/WIDTH));
            }
        }
        int p;
        for(p=1; p<comm_size; p++){  // receive the results of each process
            MPI_Status status;
            int local_roadMap[HEIGHT/comm_size + 1][WIDTH]; // array for calculated results for each process. add one row in case the workload is not evenly divided 
            int src,k;
            MPI_Recv(local_roadMap, (HEIGHT/comm_size+1)*WIDTH, MPI_INT, p, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
            src=status.MPI_SOURCE; // check the process of received message
            k=status.MPI_TAG; // check the tag of received message
            for(i=0;i<k;i++){
                for(j=0;j<WIDTH;j++){
                    roadMap[src][j]=local_roadMap[i][j];;
                }
                src+=interval;
            }
        }
        for(i=0;i<HEIGHT;i++){ 
            for(j=0;j<WIDTH;j++){
                crc += roadMap[i][j]; // get CRC from the combined roamMap
            }
        }
        dump_data();
    }
}


/**
 * Creates all the mandelbrot images and dumps them to the data directory. 
 * 
 */
void CreateMap(int my_rank, int comm_size) 
{   
    if (DO_DUMP)
        printf("xmin %.4f xmax %.4f ymin %.4f ymax %.4f\n", box_x_min, box_x_max, box_y_min, box_y_max);
    
    //divides works
    if (ROWS){
        CreateMap_Rows(my_rank, comm_size);        
    }
    
    else if (ROWSRR){
        CreateMap_RowsRR(my_rank, comm_size);        
    }
}

/**
 * Sets up the coordinate space and generates the map at different zoom level
 * 
 */
void RoadMap (int my_rank, int comm_size)
{
    int i;
    // Sets the bounding box, 
    box_x_min = -1.5; box_x_max = 0.5;
    box_y_min = -1.0; box_y_max = 1.0;

    double deltaxmin = (-0.90 - box_x_min) / zooms;
    double deltaxmax = (-0.65 - box_x_max) / zooms;
    double deltaymin = (-0.40 - box_y_min) / zooms;
    double deltaymax = (-0.10 - box_y_max) / zooms;

    // Updates the map for every zoom level
    CreateMap(my_rank, comm_size);
    for (i = 0; i < zooms; i++) {
        box_x_min += deltaxmin;
        box_x_max += deltaxmax;
        box_y_min += deltaymin;
        box_y_max += deltaymax;
        CreateMap(my_rank, comm_size);
    }                       
}

/**
 * Main function
 * 
 * @param       argc, argv      Number of command-line arguments and the arguments
 * @returns     0 
 */
int main (int argc, char *argv[])
{
    switch(argc) {
    case 2:
        if (strcmp("dump", argv[1]) == 0) {
            DO_DUMP = 1; 
        }
    case 3:
        // case: workload is divided by blocks in rows 
        if (strcmp("rows", argv[2]) == 0) { 
            ROWS = 1; 
            ROWSRR = 0;
        }
        // case: workload is divided by blocks in columns
        else if (strcmp("rowsrr", argv[2]) == 0) {
            ROWS = 0;
            ROWSRR = 1;
        }
    }
    
    double time_start = MPI_Wtime();
    
    MPI_Init(&argc, &argv);
    int my_rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    if (my_rank != 0){
        // If I am master thread, start the timer
        time_start = 0.0;
    } 
    int comm_size;
    MPI_Comm_size(MPI_COMM_WORLD, &comm_size);
    RoadMap(my_rank, comm_size); 
    MPI_Finalize();
    double time_end = MPI_Wtime();

    // Check the run time
    if (my_rank == 0){
        // If I am the master thread, write the results to a file in root (result-stat.txt/result-statRR.txt).
        char fname[256];
        FILE *result;
        
        // in case (1) row block partition
        if (ROWS==1 && ROWSRR==0){
            // Define the file name
            sprintf(fname, "result-stat.txt");
            // Open the file handle
            result = fopen(fname, "w");
            // Write the data to the file handle
            fprintf(result, "{'name' : 'roadmap_static', 'seconds' : %f, 'width' : %d, 'height' : %d, 'CRC' : 0x%x}\n", 
                (time_end - time_start), WIDTH, HEIGHT, crc);
            // Close the file handle, save the file to disk
            fclose(result); 
            // Print out the result to console 
            printf("{'name' : 'roadmap_static', 'seconds' : %f, 'width' : %d, 'height' : %d, 'CRC' : 0x%x}\n", 
                (time_end - time_start), WIDTH, HEIGHT, crc);
        }
            
        // in case (2) row cyclic partition
        else if (ROWS==0 && ROWSRR==1){
            sprintf(fname, "result-statRR.txt");
        
            // Open the file handle
            result = fopen(fname, "w");
            // Write the data to the file handle
            fprintf(result, "{'name' : 'roadmap_staticRR', 'seconds' : %f, 'width' : %d, 'height' : %d, 'CRC' : 0x%x}\n", 
                (time_end - time_start), WIDTH, HEIGHT, crc);
            // Close the file handle, save the file to disk
            fclose(result); 
            // Print out the result to console 
            printf("{'name' : 'roadmap_staticRR', 'seconds' : %f, 'width' : %d, 'height' : %d, 'CRC' : 0x%x}\n", 
                (time_end - time_start), WIDTH, HEIGHT, crc);
        }
            
    } 

    return 0;
}
