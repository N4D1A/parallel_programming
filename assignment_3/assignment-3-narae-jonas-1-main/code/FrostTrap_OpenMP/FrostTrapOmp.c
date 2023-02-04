/*********************************************************
 *                                                       *
 * Frosty Trap                                           *
 *                                                       *
 * This is actually a classic SOR kernel                 *
 *                                                       *
 * Created by Brian Vinter, June 18th 1999               *
 * Minor updates, John Markus Bjørndalen, 2016-09-15     *
 * Removed graphics, JMB 2018-09-19                      *
 *                                                       *
 ********************************************************/

/* See main() for more details about command line arguments. A short summary is
 * as follows:
 *
 * ./FrostTrap [simple|dbuf|rb] [dump]
 *
 * Where the first parameter selects the algorithm and the second parameter is
 * used to dump state to files in ./data/.
 *
 */

#include <math.h>
#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>

// Map we are building
// TODO: don't use global variables for the array etc.
// We could use a "trapdata" struct instead.

#define HEIGHT 300
#define WIDTH 300
typedef double arrtype[WIDTH][HEIGHT];
arrtype trap_data;
arrtype trap_data2;

int DO_DUMP = 0; // true if we want to dump the iterations from the file
int filenum = 0; // used to increase the file number/name in dump_data()

#ifndef _OPENMP
int omp_get_max_threads() {
    return 0;
} // for sequential version without openmp support
#endif

long long get_usecs() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec * 1000000LL + tv.tv_usec;
}

void dump_data() {
    int x, y;
    FILE *fp;
    if (!DO_DUMP)
        return;

    /* Stores the data as a Python datastructure for easy inspection and plotting
     */
    printf("dumping data\n");
    fp = fopen("data/frosttrap.data", "a+");
    for (y = 1; y < HEIGHT - 1; y++) {
        for (x = 1; x < WIDTH - 1; x++) {
            fprintf(fp, "%.2f ", trap_data[x][y]);
        }
        fprintf(fp, "\n");
    }
    fclose(fp);
}

inline double compute_new_pixel(arrtype *src, double omega, int x, int y) {
    return (omega / 4.0) * ((*src)[x][y - 1] + (*src)[x][y + 1] +
                            (*src)[x - 1][y] + (*src)[x + 1][y]) +
           (1.0 - omega) * (*src)[x][y];
}

void CreateTrap() {
    int x, y;

    // Initialize with 0 //
    for (y = 0; y < HEIGHT; y++) { 
        for (x = 0; x < WIDTH; x++) { 
            trap_data[x][y] = 0; 
        } 
    } 

    // Set up the temperature at the sides.
    for (y = 0; y < HEIGHT; y++) {
        trap_data[0][y] = -273.15;         // left side
        trap_data[WIDTH - 1][y] = -273.15; // right side
    }
    for (x = 0; x < WIDTH; x++) {
        trap_data[x][0] = 40.0;             // top
        trap_data[x][HEIGHT - 1] = -273.15; // bottom
    }
}

/* Simple SOR algorithm.
 */
int solve_simple() {
    int dumpf = 0, show_freq = 100;
    int h = HEIGHT;
    int w = WIDTH;
    double epsilon = 0.001 * h * w;
    double delta = epsilon;
    double omega = 0.8; // 0.0 < omega < 2.0
    int total_iters = 0;
        
    dump_data(); // Starting point
    while (delta >= epsilon) {
        delta = 0.0;
        # pragma omp parallel for reduction(+:delta) //default(none) shared(trap_data) firstprivate(h, w, omega)
        for (int y = 1; y < h - 1; y++) {
            for (int x = 1; x < w - 1; x++) {
                double old = trap_data[x][y];
                double new = compute_new_pixel(&trap_data, omega, x, y);
                trap_data[x][y] = new;
                delta += fabs(old - new);
            }
        }
        
        dumpf += 1;
        if (dumpf == show_freq) {
            dump_data();
            dumpf = 0;
            // printf("It %6d Delta %f epsilon %f\n", total_iters, delta, epsilon);
        }
        total_iters += 1;    
    }
    
    dump_data(); // final result
    printf("Total iterations: %d\n", total_iters);
    return total_iters;
}

/* Simple SOR algorithm, red black scheme
 */
int solve_rb() {
    int dumpf = 0, show_freq = 100;
    int h = HEIGHT;
    int w = WIDTH;
    double epsilon = 0.001 * h * w;
    double delta = epsilon;
    double omega = 0.8; // 0.0 < omega < 2.0
    int total_iters = 0;

    dump_data(); // Starting point
    while (delta >= epsilon) {
        delta = 0.0;  
        if((total_iters%2)==1){ // Update red order     
            # pragma omp parallel for reduction(+:delta) //default(none) shared(trap_data) firstprivate(h, w, omega)
            for (int y = 1; y < h - 1; y++) {            // column is odd(k=1) | even(k=2)
                for (int x = (y%2)?1:2; x < w - 1; x+=2) {  // row is odd(k=1) | even(k=2)
                    double old = trap_data[x][y];
                    double new = compute_new_pixel(&trap_data, omega, x, y);
                    trap_data[x][y] = new;
                    delta += fabs(old - new);
                }
            }
        }
        else{ // Update black order  
            # pragma omp parallel for reduction(+:delta) //default(none) shared(trap_data) firstprivate(h, w, omega)
            for (int y = 1; y < h - 1; y++) {           // column is odd(k=1) | even(k=2)
                for (int x = (y%2)?2:1; x < w - 1; x+=2) {// row is even(k=2) | odd(k=1)
                    double old = trap_data[x][y];
                    double new = compute_new_pixel(&trap_data, omega, x, y);
                    trap_data[x][y] = new;
                    delta += fabs(old - new);
                }
            }
        }
        
        dumpf += 1;
        if (dumpf == show_freq) {
            dump_data();
            dumpf = 0;
            // printf("It %6d Delta %f epsilon %f\n", total_iters, delta, epsilon);
        }
        total_iters += 1;
    }
    dump_data(); // final result
    printf("Total iterations: %d\n", total_iters);
    return total_iters;
}

/* Double buffering
 */
int solve_dbuf() {
    int dumpf = 0, show_freq = 100;
    int h = HEIGHT;
    int w = WIDTH;
    double epsilon = 0.001 * h * w;
    double delta = epsilon;
    double omega = 0.8; // 0.0 < omega < 2.0
    int total_iters = 0;

    // copy trap_data2 from trap_data
    for (int y = 1; y < h - 1; y++){ 
        for (int x = 1; x < w - 1; x++){ 
            trap_data2[x][y] = trap_data[x][y]; 
        } 
    } 
    
    dump_data(); // Starting point
    while (delta >= epsilon) {
        delta = 0.0;
        if((total_iters%2)==1){ // Update first buffer
        # pragma omp parallel for reduction(+:delta) //default(none) shared(trap_data, trap_data2) firstprivate(h, w, omega)
            for (int y = 1; y < h - 1; y++) {
                for (int x = 1; x < w - 1; x++) {
                    double old = trap_data[x][y];
                    double new = compute_new_pixel(&trap_data2, omega, x, y);
                    trap_data[x][y] = new;
                    delta += fabs(old - new);
                }
            }
        }
        else{ // Update second buffer
        # pragma omp parallel for reduction(+:delta) //default(none) shared(trap_data, trap_data2) firstprivate(h, w, omega)
            for (int y = 1; y < h - 1; y++) {
                for (int x = 1; x < w - 1; x++) {
                    double old = trap_data2[x][y];
                    double new = compute_new_pixel(&trap_data, omega, x, y);
                    trap_data2[x][y] = new;
                    delta += fabs(old - new);
                }
            }
        }
        
        dumpf += 1;
        if (dumpf == show_freq) {
            dump_data();
            dumpf = 0;
            // printf("It %6d Delta %f epsilon %f\n", total_iters, delta, epsilon);
        }
        total_iters += 1;
    }
    dump_data(); // final result
    printf("Total iterations: %d\n", total_iters);
    return total_iters;
}

int main(int argc, char **argv) {
    char *scheme = NULL;
    int (*cur_solver)() = NULL;
    char hostname[256];

    gethostname(hostname, 256);

    CreateTrap();

    // Find out which algorithm to use
    switch (argc) {
    case 3:
        if (strcmp("dump", argv[2]) == 0) {
            DO_DUMP = 1;
        }
    case 2:
        if (strcmp("simple", argv[1]) == 0) {
            cur_solver = solve_simple;
        } else if (strcmp("dbuf", argv[1]) == 0) {
            cur_solver = solve_dbuf;
        } else if (strcmp("rb", argv[1]) == 0) {
            cur_solver = solve_rb;
        } else {
            printf("Can't figure out which you want. Assuming simple.\n");
            cur_solver = solve_simple;
            scheme = "simple";
        }
        scheme = argv[1];
        break;
    default:
        scheme = "simple";
        cur_solver = solve_simple;
    }

    omp_set_num_threads(omp_get_max_threads()); //
        
    printf("Using solver %s with max-threads %d\n", scheme,
           omp_get_max_threads());
    long long t1 = get_usecs();
    int total_iters = cur_solver();
    long long t2 = get_usecs();

    // Dump something that can be parsed as python/json.
    printf("{ 'host': '%s', 'usecs': %lld, 'secs' : %f, 'scheme' : '%s', 'max_threads' : %d, 'total_iters' : %d}\n",
           hostname, t2 - t1, (t2 - t1) / 1000000.0, scheme,
           omp_get_max_threads(), total_iters);

    return 0;
}
