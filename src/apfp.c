#include <stdio.h>
#include <apfp.h>

void apfp_init(apfp_t x, apint_size_t p)
{
    apint_init(x->mant, p);
}

void apfp_free(apfp_t x)
{
    apint_free(x->mant);
}

void apfp_set_mant(apfp_ptr x, apint_size_t offset, apint_limb_t limb)
{
    apint_setlimb(x->mant, offset, limb);
}

void apfp_set_exp(apfp_ptr x, apfp_exp_t exp)
{
    x->exp = exp;
}

void apfp_print(apfp_srcptr value)
{
    printf("(");
    apint_print((apint_srcptr) &value->mant);
    printf(" * 2^");
    printf("-%lu", value->exp);
    printf(")");
}


void apfp_add(apfp_ptr x, apfp_srcptr a, apfp_srcptr b)
{
    // To-do: Handle negative values.

    // After swap, `a` is guaranteed to have largest exponent
    if (b->exp > a->exp)
    {
        apfp_srcptr t = a; a = b; b = t;
    }

    // Align `b` mantissa to `a` given exponent difference
    apfp_exp_t factor = a->exp - b->exp;
    apint_copy(x->mant, b->mant);
    apint_shiftr(x->mant, factor);

    // Add mantissa, shift by carry and update exponent
    char carry = apint_add(x->mant, x->mant, a->mant);
    apint_shiftr(x->mant, carry);
    x->exp = a->exp + carry;

    // Set the msb on the mantissa
    // To-do: Check for 0, +inf, -inf.
    apint_setmsb(x->mant);
}
