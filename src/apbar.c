#include <stdio.h>
#include <stdint.h>
#include <x86intrin.h>

#include <apbar.h>
#include <mag.h>

void rad_add(rad_ptr x, rad_srcptr a, rad_srcptr b)
{
    // derived from apfp_add
    // After swap, `a` is guaranteed to have largest exponent
    if (b->exp > a->exp)
    {
        rad_srcptr t = a; a = b; b = t;
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

uint narrow_to_rad(apfp_srcptr x, rad_ptr rad)
{
    // Essentially shift right enough so that mantissa fits into 64 bits
    uint i = x->mant->length - 1;
    for (; i >= 0; i--) {
        if (x->mant->limbs[i] != 0) {
            i = i * APINT_LIMB_BITS + APINT_LIMB_BITS - __builtin_clzl(x->mant->limbs[i]);
            break;
        }
    }

    rad->exp = x->exp + i;
}


static inline void add_error_bound(apbar_ptr res, apint_size_t prec)
{
    // From the arb paper delta y (error bound) is (|y|+n)*e
    // y = resulting midpoint (I think)
    // n = smallest representable number
    // e = machine accuracy
    apfp_t delta_y;
    apfp_init(delta_y, prec);

    // Add n. This is always just adding one to the mantissa since we are
    // rounding towards +inf.
    apint_t one;
    apint_init(one, prec);
    apint_setlimb(one, 0, 1);

    apint_add(delta_y->mant, res->midpt->mant, one);

    // Multiply be e.
    // e is 2^-p so we essentially just need to subtract p from the exponent
    delta_y->exp = res->midpt->exp - prec;

    // This is the same as arb:
    // rad_t dy;
    // dy->exp = c->rad->exp - 32;
    // dy->mant = 0x8000000000000001; // bit pattern: 1000...001

    //apbar_t t;
    //rad_add(t->rad, c->rad, dy);

    // Add delta y to the current radius
    apfp_t rad;
    apfp_init(rad, prec);
    apfp_set_mant(rad, 0, res->rad->mant);
    apfp_set_exp(rad, res->rad->exp);

    apfp_t new_rad;
    apfp_init(new_rad, prec);
    apfp_add(new_rad, delta_y, rad);

    // Narrow from apfp to fixed precision
    narrow_to_rad(new_rad, res->rad);
}

//assumes that c, a, b are already allocated
void apbar_add(apbar_ptr c, apbar_srcptr a, apbar_srcptr b, apint_size_t p)
{
    assert(a);
    assert(b);
    assert(c);

    // midpoint computation (should round towards 0)
    char is_not_exact = apfp_add(c->midpt, a->midpt, b->midpt);

    // radius computation (should round towards +inf)
    rad_add(c->rad, a->rad, b->rad);

    // error bound computation (should round towards +inf)
    if (is_not_exact) add_error_bound(c, p);
}


void apbar_mult(apbar_ptr c, apbar_srcptr a, apbar_srcptr b, apint_size_t p)
{
    assert(a);
    assert(b);
    assert(c);

    // midpoint computation (should round towards 0)
    int is_not_exact = apfp_mul(c->midpt, a->midpt, b->midpt);

    // radius computation (should round towards +inf)
    // (|x| + r)s + r|y|
    // x == a->midpt, r == a->rad
    // y == b->midpt, s == b->rad

    // For now do the computation in apfp for max precision
    // TODO: [optimisation] perform the computation in the rad type and over-estimate
    apfp_t x_abs;
    apfp_init(x_abs, p);
    apint_copy(x_abs->mant, a->midpt->mant);
    apfp_set_exp(x_abs, a->midpt->exp);
    x_abs->mant->sign = 0;

    apfp_t r;
    apfp_init(r, p);
    apfp_set_mant(r, 0, a->rad->mant);
    apfp_set_exp(r, a->rad->exp);

    apfp_t y_abs;
    apfp_init(y_abs, p);
    apint_copy(y_abs->mant, b->midpt->mant);
    apfp_set_exp(y_abs, b->midpt->exp);
    y_abs->mant->sign = 0;

    apfp_t s;
    apfp_init(s, p);
    apfp_set_mant(s, 0, b->rad->mant);
    apfp_set_exp(s, b->rad->exp);

    // TODO: Can we add numbers and have an output as one of the inputs?
    // TODO: round
    // |x| + r
    char carry = apfp_add(x_abs, x_abs, r);
    // (|x| + r) * s
    apfp_t res;
    apfp_init(res, 2*p);
    apfp_mul(res, x_abs, s);

    //r * |y|
    apfp_t res_2;
    apfp_init(res_2, 2 * p);
    apfp_mul(res_2, y_abs, r);

    carry = apfp_add(res, res, res_2);

    // narrow back to rad
    narrow_to_rad(res, c->rad);

    // error bound computation (should round towards +inf)
    if (is_not_exact) add_error_bound(c, p);
}
