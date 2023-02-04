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
int COLUMNS =0;     // true if we want to divide work by column blocks
int ROWSRR = 0;     // true if we want to divide work by row in round robin
int COLUMNSRR = 0;  // true if we want to divide work by column in round robin
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
    sprintf(fname, "data/roadmap-dyn-out-%04d.data", filenum++);
    printf("Storing data to %s.\n", fname); 
    fp = fopen(fname, "w");
    fprintf(fp, "{\n"); 
    fprintf(fp, "  'expdata' : 'roadmap-dyn',\n"); 
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

/**
 * Creates all the mandelbrot images and dumps them to the data directory. 
 * 
 */
void CreateMap(int my_rank, int comm_size, int work_rows) 
{   
    if (DO_DUMP)
        printf("xmin %.4f xmax %.4f ymin %.4f ymax %.4f\n", box_x_min, box_x_max, box_y_min, box_y_max);
    
    //divides works
    int i,j; 
    if(my_rank==0){        
        int currnet_row=0; // index of current row
        int worked_row=0; // index of worked row from each process
        int result_array[work_rows][WIDTH]; // array to receive the result of each process
        int checker=0; // check MPI_Send() and MPI_Recv() pairs
        int is_finished = -1;
    
        for(i=1;i<comm_size;i++){
            MPI_Send(&currnet_row, 1, MPI_INT, i, 0, MPI_COMM_WORLD); // send index of current work row
            checker++;
            currnet_row+=work_rows;
        }

        while(checker>0){
            MPI_Status status;
            MPI_Recv(result_array, work_rows*WIDTH, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
            checker--;
            worked_row=status.MPI_TAG;
            for(i=0;i<work_rows;i++){
                for(j=0;j<WIDTH;j++){
                    roadMap[worked_row][j]=result_array[i][j];
                }
                worked_row++;
            }
            worked_row+=work_rows;
            if(currnet_row < HEIGHT){
                MPI_Send(&currnet_row, 1, MPI_INT, status.MPI_SOURCE, 0, MPI_COMM_WORLD);
                checker++;
                currnet_row+=work_rows;
            }
            else{
                MPI_Send(&is_finished, 1, MPI_INT, status.MPI_SOURCE, 0, MPI_COMM_WORLD);
            }
        }

        for(i=0;i<HEIGHT;i++){ 
            for(j=0;j<WIDTH;j++){
                crc += roadMap[i][j]; // get CRC from the combined roamMap
            }
        }
        dump_data();
    }

    if(my_rank!=0){
        int local_roadMap[work_rows][WIDTH]; // array for calculated results for each process. 
        int k; // row count for local_roadMap array
        int working_row=0; // index of row on which each process starts work
        while(1){
            MPI_Recv(&working_row, 1, MPI_INT, 0, MPI_ANY_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            if(working_row!=-1){
                k=0;
                for (i=working_row; i<working_row+work_rows; i++){
                    for(j=0; j<WIDTH; j++){
                    local_roadMap[k][j] = solve(translate_x(((i*WIDTH)+j)%WIDTH), translate_y(((i*WIDTH)+j)/WIDTH)); 
                    }
                    k++;
                }
                MPI_Send(local_roadMap, work_rows*WIDTH, MPI_INT, 0, working_row, MPI_COMM_WORLD);
            }
            else
            {
                break;
            }          
        }
    }
}

/**
 * Sets up the coordinate space and generates the map at different zoom level
 * 
 */
void RoadMap (int my_rank, int comm_size, int work_rows)
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
    CreateMap(my_rank, comm_size, work_rows);
    for (i = 0; i < zooms; i++) {
        box_x_min += deltaxmin;
        box_x_max += deltaxmax;
        box_y_min += deltaymin;
        box_y_max += deltaymax;
        CreateMap(my_rank, comm_size, work_rows);
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
    int work_rows=1;

    switch(argc) {
    case 2:
        if (strcmp("dump", argv[1]) == 0) {
            DO_DUMP = 1; 
        }
    case 3:
    // number of rows to assign each process. default =1 
        work_rows = atoi(argv[2]);   
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
    RoadMap(my_rank, comm_size, work_rows);
    MPI_Finalize();
    double time_end = MPI_Wtime();

    if (my_rank == 0){
        // If I am the master thread, write the results to a file in root (result-dyna.txt).
        char fname[256];
        FILE *result;
        
        // Define the file name (result-dyna.txt)
        sprintf(fname, "result-dyna.txt");
        // Open the file handle
        result = fopen(fname, "w");
        // Write the data to the file handle
        fprintf(result, "{'name' : 'roadmap_dynamic', 'seconds' : %f, 'width' : %d, 'height' : %d, 'CRC' : 0x%x}\n", 
           (MPI_Wtime() - time_start), WIDTH, HEIGHT, crc);
        // Close the file handle, save the file to disk
        fclose(result); 
        // Print out the result to console 
        printf("{'name' : 'roadmap_staticRR', 'seconds' : %f, 'width' : %d, 'height' : %d, 'CRC' : 0x%x}\n", 
                (time_end - time_start), WIDTH, HEIGHT, crc);
    } 
     
    return 0;
}
