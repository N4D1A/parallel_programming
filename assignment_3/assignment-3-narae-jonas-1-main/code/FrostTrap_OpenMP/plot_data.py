#!/usr/bin/env python3
"""
Plots the results
"""

import glob
import numpy as np
import matplotlib.pyplot as plt

print("loading FrostTrap data...")
data = np.loadtxt("data/frosttrap.data")

dim = data.shape
fix, ax = plt.subplots()
for i in range(int(dim[0]/dim[1])):
    plt.imshow(data[i*dim[1]:(i+1)*dim[1]])
    plt.pause(0.001)
    ax.cla()
