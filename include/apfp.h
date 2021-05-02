#ifndef APFP_H
#define APFP_H

#include "apint.h"

typedef apint_t             apfp_mant_t;
typedef int                 apfp_sign_t; //AB: Why is sign an int?. Its also not been used anywhere??
typedef unsigned long       apfp_exp_t;

typedef struct
{
    apfp_sign_t sign;
    apfp_mant_t mant;
    apfp_exp_t  exp;
} __apfp_struct;

typedef __apfp_struct          apfp_t[1];
typedef __apfp_struct          *apfp_ptr;
typedef const __apfp_struct    *apfp_srcptr;

void apfp_init(apfp_t x, apint_size_t p);
void apfp_add(apfp_ptr x, apfp_srcptr a, apfp_srcptr b);

#endif /* !APFP_H */