#include <device_atomic_functions.hpp>

// index trap as a 2D-array 
__device__ inline float idx(float *trap, int x, int y, int w) {
    return trap[(y*w)+x]; // y = row, x = col
    //return trap[(x*w)+y];
} 

// compute pixel value
__device__ inline float compute_pixel(float *trap, float omega, int x, int y, int w) {
	return (omega / 4.0) * (idx(trap, x-1, y  , w)
						 +  idx(trap, x+1, y  , w)
						 +  idx(trap, x  , y-1, w)
						 +  idx(trap, x  , y+1, w))
						 + (1.0 - omega) * idx(trap, x,y,w);
}


/*
 * simple
 */
 __global__ void simple(float *trap, int h, int w, float omega, float epsilon, int iter, float *delta) {

    // int row = blockDim.x * blockIdx.x + threadIdx.x;
	// int col = blockDim.y * blockIdx.y + threadIdx.y;
    // int trapIdx = (row*w)+col; // consider to scale, later

    int workPerBlock = (h*w)/gridDim.x;
    for (int idx = workPerBlock*blockIdx.x + threadIdx.x; idx < workPerBlock*blockIdx.x + workPerBlock; idx += blockDim.x){
        int x = (idx%w); 
        int y = (idx/w);
        if (x > 0 && x < (w-1) && y > 0 && y < (h-1)) { 
            float oldVal = trap[idx];
            float newVal = compute_pixel(trap, omega, x, y, w);
            trap[idx] = newVal;
            atomicAdd(delta, fabs(oldVal - newVal));
        }
    }
}


/*
 * rb
 */
__global__ void rb(float *trap, int h, int w, float omega, float epsilon, int total_iter, float *delta) {
    // implement me
    //printf("%d\n", total_iter);
    int workPerBlock = (h*w)/gridDim.x;
    for (int TrapIdx = workPerBlock*blockIdx.x + threadIdx.x; TrapIdx < workPerBlock*blockIdx.x + workPerBlock; TrapIdx += blockDim.x){
        int x = (TrapIdx%w); 
        int y = (TrapIdx/w);
        if (total_iter%2){
            if (((x%2==0) && (y%2==0)) || ((x%2==1) && (y%2==1))){
                if (x > 0 && x < (w-1) && y > 0 && y < (h-1)) { 
                    float oldVal = trap[TrapIdx];
                    float newVal = compute_pixel(trap, omega, x, y, w);
                    trap[TrapIdx] = newVal;
                    atomicAdd(delta, fabsf(oldVal - newVal));
                }
            }
        }
        else{
            if (((x%2==0) && (y%2==1)) || ((x%2==1) && (y%2==0))){
                if (x > 0 && x < (w-1) && y > 0 && y < (h-1)) { 
                    float oldVal = trap[TrapIdx];
                    float newVal = compute_pixel(trap, omega, x, y, w);
                    trap[TrapIdx] = newVal;
                    atomicAdd(delta, fabsf(oldVal - newVal));
                }
            }
        }
    }
}

/*
 * dbuf
 */
__global__ void dbuf(float *trap, float *trap2, int h, int w, float omega, float epsilon, int total_iter, float *delta) {
    int workPerBlock = (h*w)/gridDim.x;
    for (int idx = workPerBlock*blockIdx.x + threadIdx.x; idx < workPerBlock*blockIdx.x + workPerBlock; idx += blockDim.x){
        int x = (idx%w); 
        int y = (idx/w);
        if (total_iter%2){
            if (x > 0 && x < (w-1) && y > 0 && y < (h-1)) { 
                float oldVal = trap2[idx];
                float newVal = compute_pixel(trap2, omega, x, y, w);
                trap[idx] = newVal;
                atomicAdd(delta, fabs(oldVal - newVal));
            }
        }
        else{
            if (x > 0 && x < (w-1) && y > 0 && y < (h-1)) { 
                float oldVal = trap[idx];
                float newVal = compute_pixel(trap, omega, x, y, w);
                trap2[idx] = newVal;
                atomicAdd(delta, fabs(oldVal - newVal));
            }
        }
    }
}

