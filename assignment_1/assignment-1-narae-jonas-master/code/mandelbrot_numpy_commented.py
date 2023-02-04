#!/usr/bin/env python3
# slightly modified (for python3) from 
# https://scipy.github.io/old-wiki/pages/Tentative_NumPy_Tutorial/Mandelbrot_Set_Example.html

from numpy import *
import pylab
 
def mandelbrot( h,w, maxit=20 ):
    '''Returns an image of the Mandelbrot fractal of size (h,w).
    '''
    #  JMB: x.shape => (1, 10), y.shape => (10, 1) (with h,w=10)
    # adding two vectors with dimensions like that creates a 10x10 matrix with the values added together.
    # as y*1j is the second vector, we automatically get complex numbers inside each cell.
    y,x = ogrid[ -1.4:1.4:h*1j, -2:0.8:w*1j ]
    c = x+y*1j
    z = c

    # this way, cells that never converge will be left as maxit. 
    divtime = maxit + zeros(z.shape, dtype=int)
    
    for i in range(maxit):
        z  = z**2 + c
        diverge = z*conj(z) > 2**2            # who is diverging  #  JMB: conj(z) inverts the sign of the imaginary part. => (a+bi)(a-bi) = a^2 + b^2 as the imaginary parts cancel each other out.  clever. 
        #  JMB: divtime is set to maxit before the loop, so (divtime==maxit) is true for all that are not yet set in divtime. => diverge & (not yet) = div_now
        div_now = diverge & (divtime==maxit)  # who is diverging now  
        divtime[div_now] = i                  # note when
        z[diverge] = 2                        # avoid diverging too much
    return divtime
         
pylab.imshow(mandelbrot(400,400))
pylab.show()
