#include "apfp.h"

#ifndef APBAR_H
#define APBAR_H

typedef apfp_t apbar_midpt; //mid point is arbitrary precision floating point
typedef unsigned long apbar_rad; //radius is fixed precision. In arb paper it is mag_t which is unsigned floating point

// mag_t datatype for radius
typedef struct
{
   // In ARB mantissa is 30 bits
   int mant;
   apfp_exp_t exp;
   apfp_sign_t sign;
} __mag_struct;

typedef __mag_struct          mag_t[1];
typedef __mag_struct          *mag_ptr;
typedef const __mag_struct    *mag_srcptr;

void mag_add(mag_ptr x, mag_srcptr a, mag_srcptr b);

typedef struct
{
    apbar_midpt midpt;
    mag_t rad;
} __apbar_struct;

typedef __apbar_struct          apbar_t[1];
typedef __apbar_struct          *apbar_ptr;
typedef const __apbar_struct    *apbar_srcptr;

void apbar_init(apbar_t x, apint_size_t p);
void apbar_add(apbar_ptr c, apbar_srcptr a, apbar_srcptr b, apint_size_t p);

#endif //APBAR_H