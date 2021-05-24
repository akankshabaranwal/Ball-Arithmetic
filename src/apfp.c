#include <stdio.h>
#include <apfp.h>
#include <flint/fmpz.h>
#include <arf.h>

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

void apfp_set_d(apfp_ptr x, double val)
{
    u_int64_t h;
    union { double uf; u_int64_t ul; } u;

    u.uf = val;
    h = u.ul;
    x->mant->sign = (int) (h >> 63);
    x->exp = (int64_t) (((h << 1) >> 53) - 1023 - 52);
    x->mant->limbs[0] = ((h << 12) >> 12) | (UWORD(1) << 52); // | 1ull<<(APINT_LIMB_BITS-1);
}

void apfp_print(apfp_srcptr value)
{
    fmpz_t exp, man;
    apint_to_fmpz(man, value->mant);
    fmpz_set_si(exp, value->exp);

    arf_t arf_val;
    arf_init(arf_val);
    arf_set_fmpz_2exp(arf_val, man, exp);
    arf_fprint(stdout, arf_val);

    arf_clear(arf_val);
    fmpz_clear(exp);
    fmpz_clear(man);

//    printf("(");
//    apint_print((apint_srcptr) &value->mant);
//    printf(" * 2^");
//    printf("%ld", value->exp);
//    printf(")");
}

char apfp_add(apfp_ptr x, apfp_srcptr a, apfp_srcptr b)
{
    apint_size_t is_not_exact;
    // After swap, `a` is guaranteed to have largest exponent
    if (b->exp > a->exp)
    {
        apfp_srcptr t = a; a = b; b = t;
    }

    apfp_ptr b_new; //b reprecisioned
    // Align `b` mantissa to `a` given exponent difference
    apfp_exp_t factor = a->exp - b->exp;
    apint_copy(x->mant, a->mant);
    apint_copy(b_new->mant, a->mant);

    apint_size_t nlimbs_new = apint_shiftl(x->mant, factor);

    if(!nlimbs_new)
    {
        reprecision(b_new->mant, nlimbs_new);
    }
    //For handling negative numbers
    char carry;
    if(a->mant->sign==b->mant->sign ) // if both have the same sign then simple add
    {
        // Add mantissa, shift by carry and update exponent
        carry = apint_plus(x->mant, x->mant, b_new->mant);
        apint_shiftr(x->mant, carry);
        x->exp = b->exp + carry;

        // Set the msb on the mantissa
        // To-do: Check for 0, +inf, -inf.
        if (carry) apint_setmsb(x->mant);
    }
    else // either a -b or b-a
    {
        apint_sub(x->mant, a->mant, b_new->mant);
        x->exp = b->exp;
        carry = 0;
    }
    return carry;
}

//a-b
//TODO: Needs to be validated
char apfp_sub(apfp_ptr x, apfp_srcptr a, apfp_srcptr b)
{
    // After swap, `a` is guaranteed to have largest exponent
    if (b->exp > a->exp)
    {
        apfp_srcptr t = a; a = b; b = t;
    }
    char is_inexact;
    // Align `b` mantissa to `a` given exponent difference
    apfp_exp_t factor = a->exp - b->exp;
    apint_copy(x->mant, a->mant);
    apint_shiftl(x->mant, factor);

    if(a->mant->sign==b->mant->sign ) // if both have the same sign then simple add
    {
        apint_sub(x->mant, a->mant, b->mant); //x->mant->sign is set here
        x->exp = b->exp;
        is_inexact = 0;
    }
    else
    {
        x->mant->sign = a->mant->sign;
        // Add mantissa, shift by carry and update exponent
        is_inexact = apint_plus(x->mant, x->mant, b->mant);
        apint_shiftr(x->mant, is_inexact);
        x->exp = b->exp + is_inexact;

        // Set the msb on the mantissa
        // To-do: Check for 0, +inf, -inf.
        if (is_inexact) apint_setmsb(x->mant);
    }

    return is_inexact;
}

void apfp_mul(apfp_ptr x, apfp_srcptr a, apfp_srcptr b)
{
    x->exp = a->exp + b->exp;
    apint_mul(x->mant, a->mant, b->mant);
    if(a->mant->sign == b->mant->sign)
    {
        x->mant->sign = a->mant->sign;
    }
    else
    {
        x->mant->sign = -1;
    }
}