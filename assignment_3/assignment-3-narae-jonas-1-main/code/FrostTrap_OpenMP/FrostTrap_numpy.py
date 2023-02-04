#!/usr/bin/env python3

"""FrostTrap using numpy arrays. 

There are two main versions of computing the next iteration of the algorithm: 
- frost_iter_loops() uses loops similar to the C version to loop over the cells and compute new values. 
  This is fairly slow. 
- frost_iter_slices() uses array slices and array operations from numpy to let numpy work on ranges of the 
  array more efficiently. This version is much closer to the C version in terms of execution speed (roughly 
  a factor 2 slower than the Python version), but it hasn't been optimized for execution speed. 

frost_trap() is used to set up and run the execution. You can specify
whether you want visualizations and with of the above step functions
you want. See the bottom of the file for examples.
"""

import time
import numpy as np
import matplotlib.pyplot as plt


class Visualizer:
    """Support object to simplify visualization of the computed FrostTrap. As a bonus, it also keeps track 
    of the iterations in the main loop.
    """
    def __init__(self, arr, freq=100, do_vis=True):
        self.arr = arr
        self.freq = freq
        self.do_vis = do_vis
        self.count = 0
        if not do_vis:
            # Trick to disable visualization (use the nop methods instead)
            self.update = self._update_nop
            self.draw   = self._draw_nop
        else:
            # Prepare visualization with matplotlib
            self.fig, self.ax = plt.subplots()
            
    def _update_nop(self):
        self.count+=1 # not quite a nop - it still keeps track of iters
        
    def update(self):
        """Updates iteration counter and draws update if we're at iter % freq == 0"""
        if (self.count % self.freq == 0):
            self.draw()
        self.count += 1
        
    def draw(self):
        plt.imshow(self.arr)
        plt.pause(0.001)
        
    def _draw_nop(self):
        pass


def create_trap(sx, sy):
    """Creates an initial array with zeroes in the middle and -237.15 along left,right and bottowm rows, 
    and 40.0 along the top row.
    """
    arr = np.zeros((sy, sx))
    arr[:,  0]  = -237.15       # left side
    arr[:, -1]  = -237.15       # right side
    arr[0, :]   =   40.00       # top
    arr[-1, :]  = -237.15       # bottom
    return arr


# This iterates over the array similarly to the C code. It's much slower than the array slice version below.
def frost_iter_loops(arr, omega=0.8):
    """Runs one iteration over the array, updating the core cells (not the edge cells)
    as a weighted average of current cell and immediate neighbours up,down,left,right.
    Omega is a weight that adjusts how much we attribute to neighbours compared to 
    the center cell. 
    """
    delta = 0.0
    for y in range(1, arr.shape[0]-1):
        for x in range(1, arr.shape[1]-1):
            nc = (omega/4.0)   * (arr[y-1][x] + arr[y+1][x] + arr[y][x-1] + arr[y][x+1]) + \
                 (1.0 - omega) * arr[y][x]
            delta += abs(nc - arr[y][x])
            arr[y][x] = nc
    return delta


# This makes use of array operations and slices in numpy (running operations on entire arrays instead of cell by cell).
# NB: no optimization has been done here (it's still fairly close to the loop version in the approach).
# The code is slightly verbose in that we extract slices first, but this is just to make it easier to read and understand. 
def frost_iter_slices(arr, omega=0.8):
    """Runs one iteration over the array, updating the core cells (not the edge cells)
    as a weighted average of current cell and immediate neighbours up,down,left,right.
    Omega is a weight that adjusts how much we attribute to neighbours compared to 
    the center cell. 
    Returns the delta (sum of difference between iterations). 
    """
    l = arr[1:-1,0:-2]          # cells left one
    r = arr[1:-1,2:,]           # cells right one
    t = arr[0:-2,1:-1]          # cells up one
    b = arr[2:,1:-1]            # cells down one
    # compute new core cells (not the static sides). 
    nc = (omega / 4)   * (l+r+t+b) + \
         (1.0 - omega) * arr[1:-1,1:-1]
    rdelta = arr[1:-1,1:-1] - nc  # difference between current and new core values
    delta = np.sum(abs(rdelta))   # absolute delta
    arr[1:-1,1:-1] = nc           # update core cells
    return delta


def frost_trap(sx, sy, do_vis=False, f_iter=frost_iter_slices, vfreq=100):
    """Creates and computes the frost trap. 
    Specify dimensions of trap with sx and sy.
    It can optionally visualize the results while running if you set do_vis to True. 
    Specify f_iter to select the function that computes the next step/iteration in the algorithm. 
    """
    print("Creating trap.")
    trap = create_trap(sx, sy)
    vis = Visualizer(trap, freq=vfreq, do_vis=do_vis)  # Initialize visualization object.
    epsilon = 0.001 * sx * sy                          # Borrowed from the c code. Used to check when we should terminate. 
    delta = epsilon + 10                               # Initial value that doesn't immediately terminate the loop.

    
    vis.draw()
    print(f"Got trap of dimension {trap.shape} and epsilon {epsilon}. Now iterating.")
    t1 = time.time()
    while delta > epsilon:
        delta = f_iter(trap)
        vis.update()
    t2 = time.time()
    vis.draw()
    print(f"Computing took {t2-t1} seconds with final delta {delta} with total {vis.count} iters.")
    return trap


if __name__ == "__main__":
    print("------ Running with numpy array slice operations, visualization ---- ")
    frost_trap(150, 150)

    print("------ Running with numpy array slice operations and visualization ---- ")
    frost_trap(150, 150, do_vis=True, vfreq=400) # 

    print("------ Running with C style loops (slower) and smaller arrays to compensate. No visualization.---- ")
    frost_trap(40,40, f_iter = frost_iter_loops)

    print("------ Running with C style loops (slower) and smaller arrays to compensate. With visualization.---- ")
    frost_trap(40,40, f_iter = frost_iter_loops, do_vis=True)
