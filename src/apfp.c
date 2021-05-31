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

void apfp_print_msg(const char *msg, apfp_srcptr value){
    printf("%s ", msg);
    apfp_print(value);
    printf("\n");
}

unsigned char apfp_add(apfp_ptr x, apfp_srcptr a, apfp_srcptr b)
{
    // After swap, `a` is guaranteed to have largest exponent
    if (b->exp > a->exp)
    {
        apfp_srcptr t = a; a = b; b = t;
    }

    // Align `b` mantissa to `a` given exponent difference
    apfp_exp_t factor = a->exp - b->exp;
    apint_copy(x->mant, b->mant);
    apint_shiftr(x->mant, factor); // right shift mantissa of b

    unsigned char overflow;

    if(a->mant->sign==b->mant->sign ) // if both have the same sign then simple add
    {
        x->mant->sign=a->mant->sign;
        // Add mantissa, shift by carry and update exponent
        overflow = apint_plus(x->mant, x->mant, a->mant); //overflow would be either 0 or 1
        apint_shiftr(x->mant, overflow);
        x->exp = a->exp + overflow;

        // Set the msb on the mantissa
        // To-do: Check for 0, +inf, -inf.
        if (overflow) apint_setmsb(x->mant);
    }
    else // either a -b or b-a
    {
        apint_sub(x->mant, a->mant, b->mant);
        overflow = apint_detectfirst1(x->mant);
        if (overflow>0)
        {
            apint_shiftl(x->mant, overflow-1);
            x->exp = a->exp - overflow;
        }
    }
    return overflow;
}

//a-b
unsigned char apfp_sub(apfp_ptr x, apfp_srcptr a, apfp_srcptr b)
{
    // After swap, `a` is guaranteed to have largest exponent
    int swapped;
    swapped = 0;
    if (b->exp > a->exp)
    {
        apfp_srcptr t = a; a = b; b = t;
        swapped=1;
    }
    unsigned char overflow;
    // Align `b` mantissa to `a` given exponent difference

    apfp_exp_t factor = a->exp - b->exp;
    apint_copy(x->mant, b->mant);
    apint_shiftr(x->mant, factor);

    if(a->mant->sign==b->mant->sign ) // if both have the same sign then simple add
    {
        apint_sub(x->mant, a->mant, x->mant); //x->mant->sign is set here
        overflow = apint_detectfirst1(x->mant);//technically this is underflow
        if(overflow>0)
        {
            apint_shiftl(x->mant, overflow);
            x->exp = a->exp - overflow;
        }

        if (overflow) apint_setmsb(x->mant); //Most likely here it is not required.
        if(swapped)
        {
            x->mant->sign = -x->mant->sign;
        }
    }
    else
    {
        x->mant->sign = a->mant->sign;
        // Add mantissa, shift by carry and update exponent
        overflow = apint_plus(x->mant, x->mant, a->mant);
        apint_shiftr(x->mant, overflow);
        x->exp = a->exp + overflow;

        // Set the msb on the mantissa
        // To-do: Check for 0, +inf, -inf.
        if (overflow) apint_setmsb(x->mant);
    }
    return overflow;
}

int apfp_mul(apfp_ptr x, apfp_srcptr a, apfp_srcptr b)
{
    x->exp = a->exp + b->exp;
    int is_exact = apint_mul(x->mant, a->mant, b->mant);
    if(a->mant->sign == b->mant->sign)
    {
        x->mant->sign = a->mant->sign;
    }
    else
    {
        x->mant->sign = -1;
    }

    //TODO: move back to left align code is left

    return is_exact;
}
