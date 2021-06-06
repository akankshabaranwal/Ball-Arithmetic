#ifndef APFP_H
#define APFP_H

#include <apint.h>
#include <stdbool.h>

typedef apint_t             apfp_mant_t;
typedef int64_t             apfp_exp_t;

// sign is only part of mantissa. otherwise there can be confusion.
typedef struct
{
    apfp_mant_t mant;
    apfp_exp_t  exp;
} __apfp_struct;

typedef __apfp_struct          apfp_t[1];
typedef __apfp_struct          *apfp_ptr;
typedef const __apfp_struct    *apfp_srcptr;

void apfp_init(apfp_t x, apint_size_t p);
void apfp_free(apfp_t x);

void apfp_set_mant(apfp_ptr x, apint_size_t offset, apint_limb_t val);
void apfp_set_mant_msb(apfp_ptr x);
apint_limb_t apfp_get_mant(apfp_srcptr x, apint_size_t offset);
void apfp_set_exp(apfp_ptr x, apfp_exp_t exp);
void apfp_set_d(apfp_ptr x, double val);
void apfp_set_pos(apfp_ptr x);
void apfp_set_neg(apfp_ptr x);

void apfp_print(apfp_srcptr value);
void apfp_print_msg(const char *msg, apfp_srcptr value);

bool apfp_add(apfp_ptr x, apfp_srcptr a, apfp_srcptr b);
bool apfp_sub(apfp_ptr x, apfp_srcptr a, apfp_srcptr b);
bool apfp_mul(apfp_ptr x, apfp_srcptr a, apfp_srcptr b);
bool apfp_mul_portable(apfp_ptr x, apfp_srcptr a, apfp_srcptr b);
bool apfp_mul_unroll(apfp_ptr x, apfp_srcptr a, apfp_srcptr b);

#endif /* !APFP_H */
