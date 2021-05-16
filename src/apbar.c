#include <stdio.h>
#include <stdint.h>
#include <x86intrin.h>

#include <apbar.h>
#include <mag.h>

void apbar_mag_add(apbar_mag_ptr x, apbar_mag_srcptr a, apbar_mag_srcptr b)
{
    // derived from apfp_add
    // After swap, `a` is guaranteed to have largest exponent
    if (b->exp > a->exp)
    {
        apbar_mag_srcptr t = a; a = b; b = t;
    }
    // Align `b` mantissa to `a` given exponent difference
    apfp_exp_t factor = a->exp - b->exp;
    x->mant = a->mant;
    x->mant <<= factor;

    uint8_t carry = _addcarryx_u64(carry, x->mant, b->mant, &x->mant);
    x->mant >>= carry;
    x->exp = b->exp + carry;

    //Set MSB
    if(carry) x->mant |= 1ull<<(APINT_LIMB_BITS-1);
}

void apbar_init(apbar_t x, apint_size_t p)
{
    apfp_init(x->midpt, p);
}

static inline void print_mid(apbar_srcptr value)
{
    apfp_print(value->midpt);
}

static inline void print_rad(apbar_srcptr value)
{
    mag_t rad;
    mag_init(rad);
    mag_set_ui_2exp_si(rad, value->rad->mant, value->rad->exp);
    mag_fprint(stdout, rad);
    mag_clear(rad);
}

void apbar_print(apbar_srcptr value) {
    print_mid(value);
    printf(" +/- ");
    print_rad(value);
}

void apbar_free(apbar_t x)
{
    apfp_free(x->midpt);
    //TODO: Need to free mag_t/ radius memory as well??
}

void apbar_set_rad(apbar_ptr x, apint_limb_t mant, apfp_exp_t exp)
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

void apbar_set_d(apbar_t x, double val)
{
    apbar_set_rad(x, 0, 0);
    apfp_set_d(x->midpt, val);
}

void apbar_add(apbar_ptr c, apbar_srcptr a, apbar_srcptr b, apint_size_t p)
{
    apbar_init(c, p);
    int is_not_exact = apfp_add(c->midpt, a->midpt, b->midpt);
    apbar_mag_add(c->rad, a->rad, b->rad);

    if (is_not_exact) {
        // From the arb paper delta y is (|y|+n)*e
        // y = resulting midpoint (I think)
        // n = smallest representable number
        // e = machine accuracy
        apfp_t delta_y;
        apfp_init(delta_y, p);

        // Add n. This is always just adding one to the mantissa since we are
        // rounding towards +inf.
        apint_t one;
        apint_init(one, p);
        apint_setlimb(one, 0, 1);

        apint_add(delta_y->mant, c->midpt->mant, one);

        // Multiply be e.
        // e is 2^-p so we essentially just need to subtract p from the exponent
        delta_y->exp = c->midpt->exp - p;


        // Add delta y to the current radius
        apfp_t rad;
        apfp_init(rad, p);
        apfp_set_mant(rad, 0, c->rad->mant);
        apfp_set_exp(rad, c->rad->exp);

        apfp_t new_rad;
        apfp_init(new_rad, p);
        apfp_add(new_rad, delta_y, rad);

        // Narrow from apfp to fixed precision
        int i = new_rad->mant->length - 1;
        for (; i >= 0; i--) {
            if (new_rad->mant->limbs[i] != 0) {
                c->rad->mant = new_rad->mant->limbs[i] + 1;
                break;
            }
        }

        c->rad->exp = new_rad->exp + (new_rad->mant->length - i) * APINT_LIMB_BITS;
    }
}
