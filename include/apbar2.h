#ifndef APBAR2_H
#define APBAR2_H

// Make sure to -DNDEBUG when benchmarking.
#include <assert.h>

#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <x86intrin.h>

#include <arf.h>

#define _IEEE_MANT_MASK ((1ull << 52) - 1ull)
#define _IEEE_EXP_MASK  ((1ull << 11) - 1ull)
#define _IEEE_EXP_SHIFT 52u
#define _IEEE_EXP_BIAS  1023u
#define _IEEE_SGN_SHIFT 63u

typedef union
{
    uint64_t    i;
    double      d;
} _apbar2_dtoi;

typedef     unsigned long long  apbar2_limb_t; // Must be defined this way for compatibility with x86intrin.h
typedef     uint64_t    apbar2_sign_t;
typedef     uint64_t    apbar2_size_t;
typedef     int64_t     apbar2_ssize_t;
typedef     int64_t     apbar2_exp_t;
typedef     double      apbar2_rad_t;

#define APBAR2_LIMB_BYTES   sizeof(apbar2_limb_t)
#define APBAR2_LIMB_BITS    (8 * APBAR2_LIMB_BYTES)
#define APBAR2_LIMB_MSBMASK (1ull << (APBAR2_LIMB_BITS - 1))

typedef struct
{
    apbar2_sign_t   sign;       // 0 for positive, 1 for negative.
    apbar2_size_t   midpt_size; // Size of the mantissa in limbs.
    apbar2_limb_t*  midpt_mant;	// Middle-left aligned.
    apbar2_exp_t    midpt_exp;
    apbar2_rad_t    rad;
} _apbar2_struct;

#define APBAR2_STRUCT_BYTES     sizeof(_apbar2_struct)

typedef _apbar2_struct          apbar2_t[1];
typedef _apbar2_struct          *apbar2_ptr;
typedef const _apbar2_struct    *apbar2_srcptr;

// Floating Point middle-alignment representation:
//
//         Upper ─┐   ┌─ Lower
//                v   v
//        xxxx xxxx | 1.xxx xxxx
//                  ^
//               Middle
//
// Note the upper part *must* contain all zeros and should only be used as padding.
#define APBAR2_LOWER(x)     ((x->midpt_size - 1) / 2)
#define APBAR2_UPPER(x)     (x->midpt_size / 2)

static inline void apbar2_init(apbar2_ptr x, apbar2_size_t prec)
{
    assert((prec % APBAR2_LIMB_BITS) == 0);

    memset(x, 0, APBAR2_STRUCT_BYTES);

    apbar2_size_t actual_size = (prec / APBAR2_LIMB_BITS) * 2;

    x->midpt_size = actual_size;
    x->midpt_mant = (apbar2_limb_t *) calloc(actual_size, APBAR2_LIMB_BYTES);
}

static inline void apbar2_free(apbar2_ptr x)
{
    assert(x->midpt_mant);
    free(x->midpt_mant);
}

static inline void apbar2_set_rad(apbar2_ptr x, apbar2_rad_t rad) { x->rad = rad; }
static inline apbar2_rad_t apbar2_get_rad(apbar2_srcptr x) { return x->rad; }

static inline void apbar2_set_midpt_exp(apbar2_ptr x, apbar2_exp_t midpt_exp)
{ x->midpt_exp = midpt_exp; }

static inline apbar2_exp_t apbar2_get_midpt_exp(apbar2_srcptr x)
{ return x->midpt_exp; }

static inline void apbar2_set_midpt_limb(apbar2_ptr x, apbar2_size_t index, apbar2_limb_t midpt_limb)
{ x->midpt_mant[index] = midpt_limb; }

static inline apbar2_limb_t apbar2_get_midpt_limb(apbar2_srcptr x, apbar2_size_t index)
{ return x->midpt_mant[index]; }

static inline void apbar2_set_sign(apbar2_ptr x, apbar2_sign_t sign) { x->sign = sign; }
static inline apbar2_sign_t apbar2_get_sign(apbar2_srcptr x) { return x->sign; }

static inline void apbar2_set_d(apbar2_ptr x, double midpt)
{
    _apbar2_dtoi uni = { .d = midpt };
    x->sign = uni.i >> _IEEE_SGN_SHIFT;
    x->midpt_mant[APBAR2_LOWER(x)] =  APBAR2_LIMB_MSBMASK |
        ((uni.i & _IEEE_MANT_MASK) << (APBAR2_LIMB_BITS - _IEEE_EXP_SHIFT - 1));
    x->midpt_exp = (apbar2_exp_t)((uni.i >> _IEEE_EXP_SHIFT) & _IEEE_EXP_MASK) - _IEEE_EXP_BIAS;
}

static inline double apbar2_get_d(apbar2_srcptr x)
{
    _apbar2_dtoi uni = { .i = 0ull };
    uni.i |= x->sign << _IEEE_SGN_SHIFT;
    uni.i |= ((x->midpt_exp + _IEEE_EXP_BIAS) & _IEEE_EXP_MASK) << _IEEE_EXP_SHIFT;
    uni.i |= (x->midpt_mant[APBAR2_LOWER(x)] >> (APBAR2_LIMB_BITS - _IEEE_EXP_SHIFT - 1)) & _IEEE_MANT_MASK;

    return uni.d;
}

static inline void _mant_to_fmpz(fmpz_t res, const apbar2_limb_t *mant, apbar2_size_t size)
{
    fmpz_init2(res, size);
    fmpz_set_ui(res, mant[0]);

    // Middle-aligned, we can skip upper-half.
    for (apbar2_size_t i = 1; i <= (size / 2); i++)
    {
        fmpz_t addend;
        fmpz_set_ui(addend, mant[i]);
        fmpz_mul_2exp(addend, addend, i * APBAR2_LIMB_BITS);
        fmpz_add(res, res, addend);
    }
}

static inline void apbar2_print(apbar2_srcptr x)
{
    if (x->sign) fprintf(stdout, "-");

    fmpz_t midpt_mant, midpt_exp;
    _mant_to_fmpz(midpt_mant, x->midpt_mant, x->midpt_size);
    fmpz_set_si(midpt_exp, x->midpt_exp - (x->midpt_size * APBAR2_LIMB_BITS / 2) + 1);

    arf_t midpt;
    arf_init(midpt);
    arf_set_fmpz_2exp(midpt, midpt_mant, midpt_exp);
    arf_fprint(stdout, midpt);

    arf_clear(midpt);
    fmpz_clear(midpt_mant);
    fmpz_clear(midpt_exp);

    fprintf(stdout, " +/- ");

    mag_t rad;
    mag_init(rad);
    mag_set_d(rad, x->rad);
    mag_fprint(stdout, rad);
    mag_clear(rad);
}

static inline void apbar2_copy(apbar2_ptr dst, apbar2_srcptr src)
{
    dst->sign = src->sign;
    dst->rad = src->rad;
    dst->midpt_exp = src->midpt_exp;

    memcpy(dst->midpt_mant, src->midpt_mant, APBAR2_LIMB_BYTES * src->midpt_size);
}

static inline double _rad_error_bound(apbar2_srcptr x, apbar2_size_t prec)
{
    int p = prec;
    double eta = ldexp(1.0, 2 - (1 << sizeof(apbar2_exp_t) * 8) - p);
    double eps = ldexp(1.0, -p);
    return (fabs(apbar2_get_d(x)) + eta) * eps;
}

// Returns detected overflow.
static inline uint8_t _add_unsigned_midpt(apbar2_ptr x, apbar2_srcptr a, apbar2_srcptr b)
{
    assert(x->midpt_mant && a->midpt_mant && b->midpt_mant);
    assert(x->midpt_size == a->midpt_size);
    assert(x->midpt_size == b->midpt_size);

    // After swap, `a' is guaranteed to have largest exponent.
    if (b->midpt_exp > a->midpt_exp)
    {
        apbar2_srcptr t = a; a = b; b = t;
    }

    // Add mantissas taking into account exponent difference.
    apbar2_exp_t shift = a->midpt_exp - b->midpt_exp;
    apbar2_size_t offset = shift / APBAR2_LIMB_BITS;
    shift -= offset * APBAR2_LIMB_BITS;

    uint8_t overflow = 0;

    apbar2_limb_t b_mant0;
    apbar2_limb_t b_mant1 = b->midpt_mant[offset];

    for (apbar2_size_t i = 0; i <= APBAR2_LOWER(x); i++)
    {
        b_mant0 = b_mant1;
        b_mant1 = b->midpt_mant[i + offset + 1];

        unsigned long long lower = b_mant0 >> shift;
        unsigned long long upper = shift ? b_mant1 << (APBAR2_LIMB_BITS - shift) : 0ULL;

        overflow = _addcarryx_u64(overflow, a->midpt_mant[i], upper | lower, &x->midpt_mant[i]);
    }

    // Update exponent in `x' accordingly.
    x->midpt_exp = a->midpt_exp + overflow;

    // Shift by one the case of an addition overflow.
    if (overflow)
    {
        apbar2_limb_t x_mant0;
        apbar2_limb_t x_mant1 = x->midpt_mant[offset];

        for (apbar2_size_t i = 0; i < APBAR2_LOWER(x); i++)
        {
            x_mant0 = x_mant1;
            x_mant1 = x->midpt_mant[offset + 1];

            x->midpt_mant[i] = (x_mant1 << (APBAR2_LIMB_BITS - 1)) | (x_mant0 >> 1u);
        }
        x->midpt_mant[APBAR2_LOWER(x)] = (x->midpt_mant[APBAR2_LOWER(x)] >> 1) | APBAR2_LIMB_MSBMASK;
    }

    return overflow;
}

static inline uint8_t _sub_unsigned_midpt(apbar2_ptr x, apbar2_srcptr a, apbar2_srcptr b)
{
    assert(x->midpt_mant && a->midpt_mant && b->midpt_mant);
    assert(x->midpt_size == a->midpt_size);
    assert(x->midpt_size == b->midpt_size);

    uint8_t swapped = 0;

    // After swap, `a' is guaranteed to have largest exponent.
    if (b->midpt_exp > a->midpt_exp)
    {
        apbar2_srcptr t = a; a = b; b = t;
        swapped = 1;
    }

    // Subtract mantissas taking into account exponent difference.
    apbar2_exp_t shift = a->midpt_exp - b->midpt_exp;
    apbar2_size_t offset = shift / APBAR2_LIMB_BITS;
    shift -= offset * APBAR2_LIMB_BITS;

    uint8_t underflow = 0;
    int last_non_zero = -1;

    for (apbar2_size_t i = 0; i <= APBAR2_LOWER(x); i++)
    {
        // To-do: Scalar replacement? Multiple array access on `x->midpt_mant[i]'.
        unsigned long long lower = b->midpt_mant[i + offset] >> shift;
        unsigned long long upper = shift ? b->midpt_mant[i + offset + 1] << (APBAR2_LIMB_BITS - shift) : 0ULL;
        underflow = _subborrow_u64(underflow, a->midpt_mant[i], upper | lower, &x->midpt_mant[i]);
        apbar2_limb_t result = x->midpt_mant[i];
        if (result)
        {
            last_non_zero = i;
        }
    }

    // Take two's complement in the case of an underflow.
    if (underflow)
    {
        uint8_t carry = 0;
        for (apbar2_size_t i = 0; i <= APBAR2_LOWER(x); i++)
        {
            // Add initial 1 in order to propagate two's complement.
            carry = _addcarryx_u64(carry, ~x->midpt_mant[i], (i == 0), &x->midpt_mant[i]);

            apbar2_limb_t result = x->midpt_mant[i];
            if (result)
            {
                last_non_zero = i;
            }
        }
    }

    // Left shift after subtraction so leading 1 is the MSB of lower-half.
    if (last_non_zero >= 0)
    {
        unsigned long long leading_zeros = _lzcnt_u64(x->midpt_mant[last_non_zero]);
        offset = APBAR2_LOWER(x) - last_non_zero;

        if (offset || leading_zeros)
        {
            for (apbar2_ssize_t i = APBAR2_LOWER(x); i >= 0; i--)
            {
                int j = i - offset;

                // To-do: Possible simplification here ?
                if (j > 0)
                {
                    x->midpt_mant[i] = (x->midpt_mant[j] << leading_zeros) |
                        (x->midpt_mant[j - 1] >> (APBAR2_LIMB_BITS - leading_zeros));
                }
                else if (j == 0)
                {
                    x->midpt_mant[i] = (x->midpt_mant[0] << leading_zeros);
                }
                else
                {
                    x->midpt_mant[i] = 0ull;
                }
            }
        }

        x->midpt_exp = a->midpt_exp - (APBAR2_LIMB_BITS * offset) - leading_zeros;
    }

    // A final underflow signifies output of *unsigned* subtraction is negative.
    return underflow ^ swapped;
}

static inline void apbar2_add(apbar2_ptr x, apbar2_srcptr a, apbar2_srcptr b, apbar2_size_t prec)
{
    uint8_t is_inexact;

    if (a->sign == b->sign)
    {
        x->sign = a->sign;
        is_inexact = _add_unsigned_midpt(x, a, b); // To-do: Perhaps mantissa alignment is also inexactitude.
    }
    else
    {
        uint8_t flipped;

        if (b->sign)
        {
            flipped = _sub_unsigned_midpt(x, a, b);
        }
        else
        {
            flipped = _sub_unsigned_midpt(x, b, a);
        }

        x->sign = (flipped) ? !a->sign : a->sign;
        is_inexact = flipped; // To-do: Perhaps underflow doesn't signify inexactitude.
    }

    // Update the radius (if is_inexact/overflow, add error bound below).
    x->rad = a->rad + b->rad;
    if (is_inexact) x->rad += _rad_error_bound(x, prec);
}

static inline void apbar2_sub(apbar2_ptr x, apbar2_srcptr a, apbar2_srcptr b, apbar2_size_t prec)
{
    // Temporarily patch b's sign to the opposite. (We have to de-const-ify...)
    apbar2_ptr b_cast = (apbar2_ptr) b;
    b_cast->sign = !b_cast->sign;
    apbar2_add(x, a, b, prec);
    b_cast->sign = !b_cast->sign;
}

static inline unsigned long long _mul_unsigned_midpt(apbar2_ptr x, apbar2_srcptr a, apbar2_srcptr b)
{
    assert(x->midpt_mant && a->midpt_mant && b->midpt_mant);
    assert(x->midpt_size == a->midpt_size);
    assert(x->midpt_size == b->midpt_size);

    unsigned long long overflow;

    for (apbar2_size_t i = 0; i <= APBAR2_UPPER(b); i++)
    {
        overflow = 0;
        for (apbar2_size_t j = 0; j <= APBAR2_UPPER(a); j++)
        {
            unsigned char carry;
            carry = _addcarryx_u64(0, x->midpt_mant[i + j], overflow, &x->midpt_mant[i + j]);
            carry |= _addcarryx_u64(0, x->midpt_mant[i + j],
                _mulx_u64(a->midpt_mant[j], b->midpt_mant[i], &overflow), &x->midpt_mant[i + j]);

            // Propagate carry in higher-up limbs
            for (apbar2_size_t k = i + j + 1; carry && (k < x->midpt_size); k++)
            {
                carry = _addcarryx_u64(carry, x->midpt_mant[k], 0, &x->midpt_mant[k]);
            }
        }
    }

    // Leading 1 is always going to land in the last limb: Right shift so it becomes the MSB of lower-half.
    // To-do: `leading_zero' can only be 1 or 0, possible optimization?
    unsigned long long leading_zero = _lzcnt_u64(x->midpt_mant[x->midpt_size - 1]);

    for (apbar2_size_t i = 0; i < x->midpt_size; i++)
    {
        if (i < APBAR2_UPPER(x))
        {
            unsigned long long lower = x->midpt_mant[APBAR2_UPPER(x) + i] << leading_zero;
            unsigned long long upper = (leading_zero) ?
                (x->midpt_mant[APBAR2_UPPER(x) + i + 1] >> (APBAR2_LIMB_BITS - leading_zero)) : 0ULL;
            x->midpt_mant[i] = upper | lower;
        } else {
            x->midpt_mant[i] = 0ull;
        }
    }

    x->midpt_exp = a->midpt_exp + b->midpt_exp + !leading_zero;

    return overflow;
}

static inline void apbar2_mul(apbar2_ptr x, apbar2_srcptr a, apbar2_srcptr b, apbar2_size_t prec)
{
    x->sign = (a->sign != b->sign);
    _mul_unsigned_midpt(x, a, b);
    x->rad = (fabs(apbar2_get_d(a)) + a->rad) * b->rad + a->rad * fabs(apbar2_get_d(b)) + _rad_error_bound(x, prec);
}

#endif //APBAR2_H
