#ifndef APBAR_H
#define APBAR_H

#include <apfp.h>

typedef apfp_t apbar_midpt; //mid point is arbitrary precision floating point
typedef unsigned long apbar_rad; //radius is fixed precision. In arb paper it is mag_t which is unsigned floating point

//sign is only part of the mantissa.
typedef struct
{
   apint_limb_t mant;
   apfp_exp_t exp;
} __rad_struct;

typedef __rad_struct          rad_t[1];
typedef __rad_struct          *rad_ptr;
typedef const __rad_struct    *rad_srcptr;

void rad_add(rad_ptr x, rad_srcptr a, rad_srcptr b);

typedef struct
{
    apbar_midpt midpt;
    rad_t rad;
} __apbar_struct;

typedef __apbar_struct          apbar_t[1];
typedef __apbar_struct          *apbar_ptr;
typedef const __apbar_struct    *apbar_srcptr;

void apbar_init(apbar_t x, apint_size_t p);
void apbar_free(apbar_t x);

void apbar_set_rad(apbar_ptr x, apint_limb_t mant, apfp_exp_t exp);
void apbar_set_midpt_exp(apbar_ptr x, apfp_exp_t exp);
apfp_exp_t apbar_get_midpt_exp(apbar_srcptr x);
void apbar_set_midpt_mant(apbar_ptr x, apint_size_t offset, apint_limb_t limb);
apint_limb_t apbar_get_midpt_mant(apbar_srcptr x, apint_size_t offset);
void apbar_set_d(apbar_ptr x, double val);

void apbar_print(apbar_srcptr value);
void apbar_print_msg(const char *msg, apbar_srcptr value);
void right_align_rad(rad_ptr rad);

void apbar_add(apbar_ptr c, apbar_srcptr a, apbar_srcptr b, apint_size_t p);
void apbar_sub(apbar_ptr c, apbar_srcptr a, apbar_srcptr b, apint_size_t p);

void apbar_mul(apbar_ptr c, apbar_srcptr a, apbar_srcptr b, apint_size_t p);
void apbar_mul_no_rad_exp(apbar_ptr c, apbar_srcptr a, apbar_srcptr b, apint_size_t p);
void apbar_mul_unroll(apbar_ptr c, apbar_srcptr a, apbar_srcptr b, apint_size_t p);

#endif //APBAR_H