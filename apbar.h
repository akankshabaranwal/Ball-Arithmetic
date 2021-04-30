#include "apfp.h"

#ifndef APBAR_H
#define APBAR_H

typedef apfp_t apbar_midpt; //mid point is arbitrary precision floating point
typedef unsigned long apbar_rad; //radius is fixed precision. In arb paper it is mag_t which is unsigned floating point

typedef struct
{
    apbar_midpt midpt;
    apbar_rad rad;
} __apbar_struct;

typedef __apbar_struct          apbar_t[1];
typedef __apbar_struct          *apbar_ptr;
typedef const __apbar_struct    *apbar_srcptr;

void apbar_init(apbar_t x, apint_size_t p);

#endif //APBAR_H
