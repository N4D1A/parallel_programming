#!/usr/bin/env python3
"""
Plots the results 

- in order to get plots when executing this file on the server you need to:
    - ssh with either -X or -Y flag
    - possibly install python3-tk package on your own computer
    - remember to change DO_DUMP to 1 in RoadMap.c file and run that file again
    - keep DO_DUMP=0 when doing performance experiments
"""

import glob
import numpy as np
from matplotlib import use
import matplotlib.pyplot as plt
use('TkAgg')

files = list(sorted(glob.glob("data/roadmap-seq-out-*.data")))

def get_data(fname):
    sdata = eval(open(fname).read())
    return np.array(sdata['arr'])

fix, ax = plt.subplots()
for i, fn in enumerate(files):
    print(fn)
    data = get_data(fn)
    #heatmap = ax.pcolor(data, cmap = plt.cm.Blues)
    #plt.imshow(data, cmap='tab20b')
    plt.imshow(data)
    plt.pause(0.01)

plt.close()


files = list(sorted(glob.glob("data/roadmap-stat-out-*.data")))

fix, ax = plt.subplots()
for i, fn in enumerate(files):
    print(fn)
    data = get_data(fn)
    #heatmap = ax.pcolor(data, cmap = plt.cm.Blues)
    #plt.imshow(data, cmap='tab20b')
    plt.imshow(data)
    plt.pause(0.01)

plt.close()


files = list(sorted(glob.glob("data/roadmap-dyn-out-*.data")))

fix, ax = plt.subplots()
for i, fn in enumerate(files):
    print(fn)
    data = get_data(fn)
    #heatmap = ax.pcolor(data, cmap = plt.cm.Blues)
    #plt.imshow(data, cmap='tab20b')
    plt.imshow(data)
    plt.pause(0.01)


# color maps
#https://matplotlib.org/examples/color/colormaps_reference.html
