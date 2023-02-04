/* ----------------------------------- mini version of complex math library ------------------- */
typedef struct complex {
    double real;
    double imag;
} complex; 

/* 
 * computes c1^2
 * Multiplication is as follows: 
 * c1*c2 = (a+bi)(c+di) = ac + adi + cbi - bd
 *   real = ac - bd
 *   imag = (ad + cb)i
 * When computing c1^2: 
 * c1^2 = (a+bi)(a+bi) 
 *      = aa + 2abi + bbi^2
 *      = aa-bb + 2abi
 *  real = aa-bb
 *  imag = 2abi
 */ 
complex complex_squared(complex c1) {
    complex c = {
        c1.real * c1.real - c1.imag * c1.imag, 
        2 * c1.real * c1.imag
    }; 
    return c; 
}

complex complex_add(complex a, complex b) {
    complex c; 
    c.real = a.real + b.real;
    c.imag = a.imag + b.imag;
    return c; 
}

// Returns magnitude squared (saves a square root operation)
float complex_magn2(complex c) {
    return c.real * c.real + c.imag * c.imag; 
}
