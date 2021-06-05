#include <stdio.h>
#include <apfp.h>
#include <flint/fmpz.h>
#include <arf.h>

void apfp_init(apfp_t x, apint_size_t p)
{
    assert(p % APINT_LIMB_BITS == 0);

    // Initialized to 2x precision for middle alignment
    apint_init(x->mant, p * 2);
}

void apfp_free(apfp_t x)
{
    apint_free(x->mant);
}

void apfp_set_mant(apfp_ptr x, apint_size_t offset, apint_limb_t limb)
{
    apint_setlimb(x->mant, offset, limb);
}

apint_limb_t apfp_get_mant(apfp_srcptr x, apint_size_t offset)
{
    return apint_getlimb(x->mant, offset);
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

    // Middle alignment: Set the "middle-right" limb to the double's mantissa
    x->mant->limbs[(x->mant->length - 1) / 2] = ((h << 12) >> 12) | (UWORD(1) << 52);
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
    assert(x->mant->length == a->mant->length);
    assert(x->mant->length == b->mant->length);

    int swapped;
    swapped=0;

    // After swap, `a` is guaranteed to have largest exponent
    if (b->exp > a->exp)
    {
        apfp_srcptr t = a; a = b; b = t;
        swapped=1;
    }

    // Align `b` mantissa to `a` given exponent difference
    apfp_exp_t factor = a->exp - b->exp;
    apint_copy(x->mant, b->mant);
    apint_shiftr(x->mant, factor); // right shift mantissa of b
    x->mant->sign = b->mant->sign;

    unsigned char overflow;

    if(a->mant->sign == b->mant->sign) // If both have the same sign then simple add
    {
        x->mant->sign=a->mant->sign;
        // Add mantissa, shift by carry and update exponent
        apint_plus(x->mant, x->mant, a->mant);

        overflow = x->mant->limbs[x->mant->length / 2]; // Middle-left limb has first bit set -> overflow occured
        assert(overflow == 0 || overflow == 1); // Extra assertion for safety (should be 0 or 1, but not more)

        apint_shiftr(x->mant, overflow);
        x->exp = a->exp + overflow;

        // Force the msb on the middle-right limb
        x->mant->limbs[(x->mant->length - 1) / 2] |= (1llu << (APINT_LIMB_BITS - 1));
    }
    else // Either (a - b) or (b - a)
    {
        apint_sub(x->mant, a->mant, x->mant);
        overflow = x->mant->limbs[x->mant->length / 2]; // Middle-left limb has all 1's -> underflow occured
        assert(overflow == 0 || overflow == (apint_limb_t)(-1)); // Extra assertion for safety (should be 0 or all 1s)

        if (overflow)
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
