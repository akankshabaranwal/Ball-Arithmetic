#include "apbar.h"

void mag_add(mag_ptr x, mag_srcptr a, mag_srcptr b)
{
    // derived from apfp_add
    // Todo: validate this
    // After swap, `a` is guaranteed to have largest exponent
    if (b->exp > a->exp)
    {
        mag_srcptr t = a; a = b; b = t;
    }
    // Align `b` mantissa to `a` given exponent difference
    apfp_exp_t factor = a->exp - b->exp;
    x->mant = b->mant;
    x->mant = x->mant>>factor;

    // Todo: check if we need to handle carry here?
    x->mant = x->mant + a->mant;
    x->exp = a->exp;
}

void apbar_init(apbar_t x, apint_size_t p)
{    apfp_init(x->midpt, p);
}

void apbar_add(apbar_ptr c, apbar_srcptr a, apbar_srcptr b, apint_size_t p)
{
    apbar_init(c, p);
    apfp_add(c->midpt, a->midpt, b->midpt);
    mag_add(c->rad, a->rad, b->rad);
}