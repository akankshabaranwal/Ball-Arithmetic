#include "apbar.h"

void mag_add(mag_ptr x, mag_srcptr a, mag_srcptr b)
{
    // derived from apfp_add
    // After swap, `a` is guaranteed to have largest exponent
    if (b->exp > a->exp)
    {
        mag_srcptr t = a; a = b; b = t;
    }
    // Align `b` mantissa to `a` given exponent difference
    apfp_exp_t factor = a->exp - b->exp;
    x->mant = b->mant;
    x->mant = x->mant>>factor;
    char carry =0;
    carry = _addcarryx_u64(carry, a->mant, b->mant, &x->mant);
    x->mant>>carry;
    x->exp = a->exp + carry;
    //x->mant = x->mant + a->mant;
    //x->exp = a->exp;
}

void apbar_init(apbar_t x, apint_size_t p)
{    apfp_init(x->midpt, p);
}

void apbar_set_rad(apbar_ptr x, int mant, apfp_exp_t exp)
{
    x->rad->mant = mant;
    x->rad->exp = exp;
}

void apbar_set_midpt_exp(apbar_ptr x, apfp_exp_t exp)
{
    apfp_set_exp(x->midpt, exp);
}

void apbar_set_midpt_mant(apbar_ptr x, apint_size_t offset, apint_limb_t limb)
{
    apfp_set_mant(x->midpt, offset, limb);
}

void apbar_add(apbar_ptr c, apbar_srcptr a, apbar_srcptr b, apint_size_t p)
{
    apbar_init(c, p);
    apfp_add(c->midpt, a->midpt, b->midpt);
    mag_add(c->rad, a->rad, b->rad);
}