#include "apfp.h"

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
