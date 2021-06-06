#include <stdio.h>
#include <stdint.h>
#include <x86intrin.h>
#include <stdbool.h>

#include <apbar.h>
#include <mag.h>

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

void apbar_print_msg(const char *msg, apbar_srcptr value) {
    printf("%s ", msg);
    apbar_print(value);
    printf("\n");
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

apfp_exp_t apbar_get_midpt_exp(apbar_srcptr x)
{
    return x->midpt->exp;
}

void apbar_set_midpt_mant(apbar_ptr x, apint_size_t offset, apint_limb_t limb)
{
    apfp_set_mant(x->midpt, offset, limb);
}

apint_limb_t apbar_get_midpt_mant(apbar_srcptr x, apint_size_t offset)
{
    return apfp_get_mant(x->midpt, offset);
}

void apbar_set_d(apbar_t x, double val)
{
    apbar_set_rad(x, 0, 0);
    apfp_set_d(x->midpt, val);
}

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
    if (factor >= APINT_LIMB_BITS) {
        x->mant++;
        x->exp = a->exp;
        // printf("(%ld) %llu * 2^%ld + %llu * 2^%ld = %llu * 2^%ld\n", factor, a->mant, a->exp, b->mant, b->exp, x->mant, x->exp);
        return;
    }
    x->mant <<= factor;

    uint8_t carry = _addcarryx_u64(carry, x->mant, b->mant, &x->mant);
    x->mant >>= carry;
    x->exp = b->exp + carry;

    //Set MSB
    if(carry) x->mant |= 1ull<<(APINT_LIMB_BITS-1);
    // printf("%llu * 2^%ld + %llu * 2^%ld = %llu * 2^%ld\n", a->mant, a->exp, b->mant, b->exp, x->mant, x->exp);
}

void narrow_to_rad(apfp_ptr x, rad_ptr rad)
{
    // Essentially shift right enough so that mantissa fits into 64 bits
    size_t pos = apint_detectfirst1(x->mant);
    size_t shift = pos - APINT_LIMB_BITS;
    apint_shiftr(x->mant, shift);
    rad->mant = x->mant->limbs[0];
    // if (shift > 0) rad->mant++; <-- for rounding, when we shift off bits (did we shift bits?)
    rad->exp = x->exp + shift;
}

void right_align_rad(rad_ptr rad)
{
    while((rad->mant & 0x1llu) == 0) {
        rad->mant >>= 1;
        rad->exp++;
    }
}

static inline void expand_rad(apfp_ptr fp, rad_srcptr rad)
{
    uint leading_zeros = __builtin_clzl(rad->mant);
    uint limb_pos = fp->mant->length / 2 - 1;
    fp->mant->limbs[limb_pos] = rad->mant << leading_zeros;

    fp->exp = rad->exp - (limb_pos) * APINT_LIMB_BITS - leading_zeros ;
    apfp_set_pos(fp);
}

static inline void add_error_bound(apbar_ptr res, apint_size_t prec)
{
    // From the arb paper delta y (error bound) is (|y|+n)*e
    // y = resulting midpoint (I think)
    // n = smallest representable number
    // e = machine accuracy
    apint_size_t res_length = res->midpt->mant->length * APINT_LIMB_BITS;
    apfp_t delta_y;
    apfp_init(delta_y, res_length);

    // Add n. This is always just adding one to the mantissa since we are
    // rounding towards +inf.
    apint_t one;
    apint_init(one, res_length);
    apint_setlimb(one, 0, 1);

    unsigned char carry = apint_add(delta_y->mant, res->midpt->mant, one);

    // Multiply by e.
    // e is 2^-p so we essentially just need to subtract p from the exponent
    delta_y->exp = res->midpt->exp - prec - carry;
    if (carry) apint_setmsb(delta_y->mant);
    apfp_set_pos(delta_y);

    // Add delta y to the current radius
    apfp_t rad;
    apfp_init(rad, res_length);
    expand_rad(rad, res->rad);

    apfp_t new_rad;
    apfp_init(new_rad, res_length);
    apfp_add(new_rad, delta_y, rad);

    // Narrow from apfp to fixed precision
    narrow_to_rad(new_rad, res->rad);

    apfp_free(rad);
    apfp_free(new_rad);
    apfp_free(delta_y);

    // This is almost the same as arb:
//     rad_t dy;
//     dy->exp = res->rad->exp - 32;
//     dy->mant = 0x8000000000000001; // bit pattern: 1000...001
//
//    rad_t t;
//    rad_add(t, res->rad, dy);
//    printf("t is: %llu * 2^%ld\n", t->mant, t->exp);
}

//assumes that c, a, b are already allocated
// This is not the complete base
// Maybe have different plots showing how optimizing shiftl or shiftr improved the performance etc for the base case
void apbar_add_base(apbar_ptr c, apbar_srcptr a, apbar_srcptr b, apint_size_t p)
{
    assert(a);
    assert(b);
    assert(c);

    // midpoint computation (should round towards 0)
    bool is_exact = apfp_add_base(c->midpt, a->midpt, b->midpt);

    // radius computation (should round towards +inf)
    rad_add(c->rad, a->rad, b->rad);

    // error bound computation (should round towards +inf)
    if (!is_exact) add_error_bound(c, p);
}

// apbar_add shiftl base v/s shiftl optimal??
// apbar add shiftr base v/s shiftr optimal??

void apbar_add_optim1(apbar_ptr c, apbar_srcptr a, apbar_srcptr b, apint_size_t p)
{
    assert(a);
    assert(b);
    assert(c);

    // midpoint computation (should round towards 0)
    bool is_exact = apfp_add_optim1(c->midpt, a->midpt, b->midpt);

    // radius computation (should round towards +inf)
    rad_add(c->rad, a->rad, b->rad);

    // error bound computation (should round towards +inf)
    if (!is_exact) add_error_bound(c, p);
}

void apbar_add(apbar_ptr c, apbar_srcptr a, apbar_srcptr b, apint_size_t p)
{
    assert(a);
    assert(b);
    assert(c);

    // midpoint computation (should round towards 0)
    bool is_exact = apfp_add(c->midpt, a->midpt, b->midpt);

    // radius computation (should round towards +inf)
    rad_add(c->rad, a->rad, b->rad);

    // error bound computation (should round towards +inf)
    if (!is_exact) add_error_bound(c, p);
}

//assumes that c, a, b are already allocated
void apbar_sub(apbar_ptr c, apbar_srcptr a, apbar_srcptr b, apint_size_t p)
{
    assert(a);
    assert(b);
    assert(c);

    // midpoint computation (should round towards 0)
    bool is_exact = apfp_sub(c->midpt, a->midpt, b->midpt);

    // radius computation (should round towards +inf)
    rad_add(c->rad, a->rad, b->rad);

    // error bound computation (should round towards +inf)
    if (!is_exact) add_error_bound(c, p);
}

void apbar_mul(apbar_ptr c, apbar_srcptr a, apbar_srcptr b, apint_size_t p)
{
    assert(a);
    assert(b);
    assert(c);

    // midpoint computation (should round towards 0)
    bool is_exact = apfp_mul(c->midpt, a->midpt, b->midpt);

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
    apfp_set_pos(x_abs);

    apfp_t r;
    apfp_init(r, p);
    expand_rad(r, a->rad);

    apfp_t y_abs;
    apfp_init(y_abs, p);
    apint_copy(y_abs->mant, b->midpt->mant);
    apfp_set_exp(y_abs, b->midpt->exp);
    apfp_set_pos(y_abs);

    apfp_t s;
    apfp_init(s, p);
    expand_rad(s, b->rad);

    apint_t one;
    apint_init(one, 2*p);
    apint_setlimb(one, 0, 1);

    bool is_exact_sub;
    // TODO: Can we add numbers and have an output as one of the inputs?
    // TODO: round
    // |x| + r
    is_exact_sub = apfp_add(x_abs, x_abs, r);
    if (!is_exact_sub) apint_add(x_abs->mant, x_abs->mant, one);
    // (|x| + r) * s
    apfp_t res;
    apfp_init(res, p);
    is_exact_sub = apfp_mul(res, x_abs, s);
    if (!is_exact_sub) apint_add(res->mant, res->mant, one);

    //r * |y|
    apfp_t res_2;
    apfp_init(res_2, p);
    is_exact_sub = apfp_mul(res_2, y_abs, r);
    if (!is_exact_sub) apint_add(res_2->mant, res_2->mant, one);

    is_exact_sub = apfp_add(res, res, res_2);
    if (!is_exact_sub) apint_add(res->mant, res->mant, one);

    // narrow back to rad
    narrow_to_rad(res, c->rad);

    apfp_free(res_2);
    apfp_free(res);
    apfp_free(s);
    apfp_free(y_abs);
    apfp_free(r);
    apfp_free(x_abs);

    // error bound computation (should round towards +inf)
    if (!is_exact) add_error_bound(c, p);
}

static inline void narrow_to_rad_keep(apfp_ptr x, rad_ptr rad)
{
    // Essentially shift right enough so that mantissa fits into 64 bits
    size_t pos = apint_detectfirst1(x->mant);
    size_t shift = pos - APINT_LIMB_BITS;
    apint_t new_mant;
    apint_init(new_mant, x->mant->length * APINT_LIMB_BITS);
    apint_shiftr_copy(new_mant, x->mant, shift);
    rad->mant = new_mant->limbs[0] + 1;
    rad->exp = x->exp + shift;
}

static inline void rad_mul(rad_ptr c, rad_srcptr a, rad_srcptr b)
{
    c->exp = a->exp + b->exp;
    uint64_t low = _mulx_u64(a->mant, b->mant, &c->mant);
    uint shift = __builtin_clzl(c->mant);
    c->mant = (c->mant << shift) + ((apint_limb_t) low >> (APINT_LIMB_BITS - shift)) + 1;
    c->exp += (apfp_exp_t) APINT_LIMB_BITS - shift;
}

static inline void error_bound_no_exp(apbar_ptr res, apint_size_t prec)
{
    // From the arb paper delta y (error bound) is (|y|+n)*e
    // y = resulting midpoint (I think)
    // n = smallest representable number
    // e = machine accuracy
    rad_t delta_y;
    narrow_to_rad_keep(res->midpt, delta_y);

    // Add n. This is always just adding one to the mantissa since we are
    // rounding towards +inf.
    delta_y->mant++;

    // Multiply by e.
    // e is 2^-p so we essentially just need to subtract p from the exponent
    delta_y->exp -= prec;

    // Add delta y to the current radius
    rad_add(res->rad, res->rad, delta_y);
}

void apbar_mul_no_rad_exp(apbar_ptr c, apbar_srcptr a, apbar_srcptr b, apint_size_t p)
{
    assert(a);
    assert(b);
    assert(c);

    // midpoint computation (should round towards 0)
    bool is_exact = apfp_mul(c->midpt, a->midpt, b->midpt);

    // radius computation (should round towards +inf)
    // (|x| + r)s + r|y|
    // x == a->midpt, r == a->rad
    // y == b->midpt, s == b->rad

    // For now do the computation in apfp for max precision
    // TODO: [optimisation] perform the computation in the rad type and over-estimate
    rad_t x_abs;
    narrow_to_rad_keep(a->midpt, x_abs);

    rad_t y_abs;
    narrow_to_rad_keep(b->midpt, y_abs);

    // TODO: Can we add numbers and have an output as one of the inputs?
    // TODO: round
    // |x| + r
    rad_add(x_abs, x_abs, a->rad);
    // (|x| + r) * s
    rad_mul(x_abs, x_abs, b->rad);

    //r * |y|
    rad_mul(y_abs, y_abs, a->rad);

    rad_add(c->rad, x_abs, y_abs);

    // error bound computation (should round towards +inf)
    if (!is_exact) error_bound_no_exp(c, p);
}


void apbar_mul_unroll(apbar_ptr c, apbar_srcptr a, apbar_srcptr b, apint_size_t p)
{
    assert(a);
    assert(b);
    assert(c);

    // midpoint computation (should round towards 0)
    bool is_exact = apfp_mul_unroll(c->midpt, a->midpt, b->midpt);

    // radius computation (should round towards +inf)
    // (|x| + r)s + r|y|
    // x == a->midpt, r == a->rad
    // y == b->midpt, s == b->rad

    // For now do the computation in apfp for max precision
    // TODO: [optimisation] perform the computation in the rad type and over-estimate
    rad_t x_abs;
    narrow_to_rad_keep(a->midpt, x_abs);

    rad_t y_abs;
    narrow_to_rad_keep(b->midpt, y_abs);

    // TODO: Can we add numbers and have an output as one of the inputs?
    // TODO: round
    // |x| + r
    rad_add(x_abs, x_abs, a->rad);
    // (|x| + r) * s
    rad_mul(x_abs, x_abs, b->rad);

    //r * |y|
    rad_mul(y_abs, y_abs, a->rad);

    rad_add(c->rad, x_abs, y_abs);

    // error bound computation (should round towards +inf)
    if (!is_exact) error_bound_no_exp(c, p);
}
