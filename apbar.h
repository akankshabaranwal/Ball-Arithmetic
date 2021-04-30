#include "apfp.h"

#ifndef APBAR_H
#define APBAR_H

typedef apfp_t apbar_midpt; //mid point is arbitrary precision floating point
typedef float apbar_rad; //radius is fixed precision. In arb paper it is mag_t which is unsigned floating point

typedef struct
{
    apbar_rad rad;
    apbar_midpt midpt;
} __apbar_struct;

typedef __apbar_struct          apbar_t[1];
typedef __apbar_struct          *apbar_ptr;
typedef const __apbar_struct    *apbar_srcptr;

//TODO: Implement the precision parameter
void apbar_add(apbar_t a, apbar_t b, apbar_t c, apbar_t p);

#endif //APBAR_H
