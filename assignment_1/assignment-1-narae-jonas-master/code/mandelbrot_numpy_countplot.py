#!/usr/bin/env python3
# based on
# https://scipy.github.io/old-wiki/pages/Tentative_NumPy_Tutorial/Mandelbrot_Set_Example.html
# and
# http://matplotlib.org/examples/pylab_examples/scatter_hist.html
# We can't use histograms as they count the wrong thing. We use bar plots instead.
# Histograms would make more sense for monte carlo pi. 

from numpy import *
import pylab
import matplotlib.pyplot as plt
from matplotlib.ticker import NullFormatter
import numpy as np
 
def mandelbrot( h,w, maxit=20 ):
    '''Returns an image of the Mandelbrot fractal of size (h,w).
    '''
    y,x = ogrid[ -1.4:1.4:h*1j, -2.0:0.8:w*1j ]
    c = x+y*1j
    z = c
    divtime = maxit + zeros(z.shape, dtype=int)

    for i in range(maxit):
        z  = z**2 + c
        diverge = z*conj(z) > 2**2            # who is diverging
        div_now = diverge & (divtime==maxit)  # who is diverging now
        divtime[div_now] = i                  # note when
        z[diverge] = 2                        # avoid diverging too much
    return divtime

def plot_view(img, x_sums, y_sums):
    nullfmt = NullFormatter()         # no labels

    # definitions for the axes
    left, width = 0.1, 0.65
    bottom, height = 0.1, 0.65
    bottom_h = left_h = left + width + 0.02

    rect_scatter = [left, bottom, width, height]
    rect_barx = [left, bottom_h, width, 0.2]
    rect_bary = [left_h, bottom, 0.2, height]

    # start with a rectangular Figure
    plt.figure(1, figsize=(8, 8))

    axImg = plt.axes(rect_scatter)
    axBarx = plt.axes(rect_barx)
    axBary = plt.axes(rect_bary)

    # no labels
    axBarx.xaxis.set_major_formatter(nullfmt)
    axBary.yaxis.set_major_formatter(nullfmt)

    # the img plot
    axImg.imshow(img)
    # now the bar plot showing the total iterations along that row/column
    axBarx.bar(np.arange(0, x_sums.shape[0]), x_sums)
    axBary.barh(np.arange(0, y_sums.shape[0]), y_sums)

    for tick in axBary.xaxis.get_ticklabels():
        tick.set_rotation(45)

    axBarx.set_xlim(axImg.get_xlim())
    axBary.set_ylim(axImg.get_ylim())

    plt.savefig("tmp.jpg")
    plt.show()
    

img =  mandelbrot(400,400)
x_sums = img.sum(0)  # sums along x-axis
y_sums = img.sum(1)
plot_view(img, x_sums, y_sums)

