#include <stdio.h>
#include <math.h>
#include "complex_lib.h"
#include <sys/time.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#define WIDTH 2000
#define HEIGHT 2000

// The maximum number of iterations we will compute before giving up at a given coordinate
// This used to be 1024, but that masked out all the variation between 1-100, where most of the details are. 
#define MAX_ITERATIONS 100

int DO_DUMP = 0;    // true if we want to dump the iterations from the file
int zooms = 10;     // number of zooms before we stop
int crc = 0;        // used for debugging (compare parallel with sequential, for instance)

int roadMap[HEIGHT][WIDTH];

double box_x_min, box_x_max, box_y_min, box_y_max;

long long get_usecs()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec * 1000000LL + tv.tv_usec; 
}


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
    
    /* Stores the data as a Python datastructure for easy inspection and plotting */ 
    sprintf(fname, "data/roadmap-seq-out-%04d.data", filenum++);
    printf("Storing data to %s.\n", fname); 
    fp = fopen(fname, "w");
    fprintf(fp, "{\n"); 
    fprintf(fp, "  'expdata' : 'roadmap-seq',\n"); 
    fprintf(fp, "  'arr'     : [\n"); 
    int y, x;
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
void CreateMap() 
{
    int x, y;

    if (DO_DUMP)
        printf("xmin %.4f xmax %.4f ymin %.4f ymax %.4f\n", box_x_min, box_x_max, box_y_min, box_y_max);
    
    //Loops over pixels
    for (y=0; y<HEIGHT; y++) {
        for (x=0; x<WIDTH; x++) {  
            // Store the number of iterations for this pixel
            int c = solve(translate_x(x), translate_y(y)); 
            roadMap[y][x] = c;
            crc += c; 
        }
    }
    dump_data(); 
}

/**
 * Sets up the coordinate space and generates the map at different zoom level
 * 
 */
void RoadMap ()
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
    CreateMap();
    for (i = 0; i < zooms; i++) {
        box_x_min += deltaxmin;
        box_x_max += deltaxmax;
        box_y_min += deltaymin;
        box_y_max += deltaymax;
        CreateMap();
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
    }
    
    long long t1 = get_usecs(); 
    RoadMap();
    long long t2 = get_usecs(); 

    printf("{'name' : 'roadmap_seq', 'usecs' : %lld, 'secs' : %f, 'width' : %d, 'height' : %d, 'CRC' : 0x%x}\n", 
           t2-t1, (t2-t1)/1000000.0, WIDTH, HEIGHT, crc); 
    return 0;
}
