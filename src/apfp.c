#include <stdio.h>
#include <apfp.h>
#include <flint/fmpz.h>
#include <arf.h>

#define MIDDLE_LEFT(NUM) NUM->mant->limbs[NUM->mant->length / 2]
#define MIDDLE_RIGHT(NUM) NUM->mant->limbs[NUM->mant->length / 2 - 1]
#define MID_POS(NUM) NUM->mant->length / 2
#define MID_POS_BITWISE(NUM) MID_POS(NUM) * APINT_LIMB_BITS

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
    assert(offset < x->mant->length / 2);
    apint_setlimb(x->mant, offset, limb);
}

void apfp_set_mant_msb(apfp_ptr x)
{
    MIDDLE_RIGHT(x) = ((apint_limb_t)1 << (sizeof(apint_limb_t) * 8 - 1));
}

apint_limb_t apfp_get_mant(apfp_srcptr x, apint_size_t offset)
{
    assert(offset < x->mant->length / 2);
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
    MIDDLE_RIGHT(x) = ((h << 12) >> 12) | (UWORD(1) << 52);
}

void apfp_set_pos(apfp_ptr x)
{
    x->mant->sign = 1;
}

void apfp_set_neg(apfp_ptr x)
{
    x->mant->sign = -1;
}

void apfp_print(apfp_srcptr value)
{
    if (value->mant->sign < 0) {
        printf("-");
    }
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
}

void apfp_print_msg(const char *msg, apfp_srcptr value){
    printf("%s ", msg);
    apfp_print(value);
    printf("\n");
}

static inline bool adjust_alignment(apfp_ptr x)
{
    bool is_exact = true;
    size_t overflow = apint_detectfirst1(x->mant);

    if (overflow > MID_POS_BITWISE(x))
    {
        overflow -= MID_POS_BITWISE(x);
        is_exact = apint_shiftr(x->mant, overflow);
        x->exp += (apfp_exp_t) overflow;
    }
    else if (overflow < MID_POS_BITWISE(x))
    {
        // Can't shift off bits here
        overflow = MID_POS_BITWISE(x) - overflow;
        apint_shiftl(x->mant, overflow);
        x->exp -= (apfp_exp_t) overflow;
    }
    return is_exact;
}

bool apfp_add(apfp_ptr x, apfp_srcptr a, apfp_srcptr b)
{
    assert(x->mant->length == a->mant->length);
    assert(x->mant->length == b->mant->length);

    bool swapped = false;
    bool is_exact = true;
    // After swap, `a` is guaranteed to have largest exponent
    if (b->exp > a->exp)
    {
        apfp_srcptr t = a; a = b; b = t;
        swapped = true;
    }

    // Align `b` mantissa to `a` given exponent difference
    apfp_exp_t factor = a->exp - b->exp;

    // We could easily combine shift and copy here
    apint_copy(x->mant, b->mant);
    apint_shiftr(x->mant, factor); // right shift mantissa of b
    x->mant->sign = b->mant->sign;

    // Add mantissa, shift by carry and update exponent
    apint_add(x->mant, x->mant, a->mant);
    x->exp = a->exp;
    if(MIDDLE_LEFT(x) != 0 && (apint_getlimb(x->mant, 0) & 0x1ull) != 0)
        is_exact = false;

    adjust_alignment(x);

    return is_exact;
}

//a-b
bool apfp_sub(apfp_ptr x, apfp_srcptr a, apfp_srcptr b)
{
    // After swap, `a` is guaranteed to have largest exponent
    bool swapped = false;
    bool is_exact = true;
    if (b->exp > a->exp)
    {
        apfp_srcptr t = a; a = b; b = t;
        swapped = true;
    }
    // Align `b` mantissa to `a` given exponent difference
    apfp_exp_t factor = a->exp - b->exp;
    apint_copy(x->mant, b->mant);
    apint_shiftr(x->mant, factor);

    if(x->mant->sign == b->mant->sign ) // if both have the same sign then simple add
    {
        // Subtract the two mantissas
        apint_sub(x->mant, a->mant, x->mant); //x->mant->sign is set here
        if(swapped)
        {
            x->mant->sign = -x->mant->sign;
        }
    }
    else
    {
        // Add the two mantissas
        x->mant->sign = a->mant->sign;
        apint_plus(x->mant, x->mant, a->mant);
    }
    x->exp = a->exp;
    if(MIDDLE_LEFT(x) !=0 && (apint_getlimb(x->mant, 0) & 0x1ull) != 0)
        is_exact = false;
    adjust_alignment(x);

    return is_exact;
}

bool apfp_mul(apfp_ptr x, apfp_srcptr a, apfp_srcptr b)
{
    x->exp = a->exp + b->exp;
    apint_mul(x->mant, a->mant, b->mant);
    adjust_alignment(x);

    if(a->mant->sign == b->mant->sign)
    {
        apfp_set_pos(x);
    }
    else
    {
        apfp_set_neg(x);
    }

    return ;
}
