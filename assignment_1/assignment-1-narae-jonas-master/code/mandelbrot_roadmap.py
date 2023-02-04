#!/usr/bin/env python3
# Mandelbrot using python and matplotlib. This is based on the RoadMap.c code. 
# Python has direct support for complex numbers, simplifying the computation. 
import time
import matplotlib.pyplot as plt

MAX_ITER = 256
MAX_ITER = 20  # 20 looks better than 256 for some reason. Guess it's the colour mapping. 

def calc_pixel(c):
    "Returns the iteration count for the complex coordinate (c)."
    z = 0
    for i in range(MAX_ITER):
        z = z ** 2 + c
        if (z.real ** 2 + z.imag ** 2) > 4:
            return i # magnitude larger than 2
    return i

def get_zoom(n):
    """Computes a new zoom/visualisation box. Based on the C code (RoadMap.c)"""
    zooms = 10.0
    x_min = -1.5
    x_max =  0.5
    y_min = -1.0
    y_max =  1.0
    dxmin = (-0.90 - x_min) / zooms
    dxmax = (-0.65 - x_max) / zooms
    dymin = (-0.40 - y_min) / zooms
    dymax = (-0.10 - y_max) / zooms
    
    x_min += n * dxmin
    x_max += n * dxmax
    y_min += n * dymin
    y_max += n * dymax
    return (x_min, x_max, y_min, y_max)

def trans_coord(box, x, y, nx, ny):
    "Returns a complex coordinate inside the zoom box based on x,y <= nx,ny "
    xr = (x * (box[1]-box[0]) / nx) + box[0]
    yr = (y * (box[3]-box[2]) / nx) + box[2]
    return xr + yr * 1j

def compute_mbrot(box, size_x, size_y):
    "Returns a 2D array with iteration counts for each of the pixels in the specified box" 
    print("Starting compute")
    t1 = time.time()
    data = []
    for y in range(size_x):
        row = []
        for x in range(size_y):
            row.append(calc_pixel(trans_coord(box, x, y, size_x, size_y)))
        data.append(row)
    t2 = time.time()
    print(" - done using {} seconds".format(t2-t1))
    return data
    

# simple test
# box = get_zoom(0)
# c = trans_coord(box, 30, 50, 100, 100)
# count = calc_pixel(c)

for n in range(10):
    box = get_zoom(n)
    data = compute_mbrot(box, 100, 100)

    # visualise, method 1
    fix, ax = plt.subplots()
    heatmap = ax.pcolor(data, cmap = plt.cm.Blues)
    plt.show()

    # visualise, method 2a
    #plt.imshow(data, cmap='hot', interpolation='nearest')
    plt.imshow(data)
    plt.show()
