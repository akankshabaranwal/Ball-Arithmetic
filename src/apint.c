#include <assert.h>
#include <x86intrin.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include <apint.h>
#include <flint/fmpz.h>

void apint_init(apint_t x, apint_size_t p)
{
    x->length = (p / APINT_LIMB_BITS) + ((p % APINT_LIMB_BITS) > 0);
    x->limbs = malloc(x->length * APINT_LIMB_BYTES);
}

void apint_to_fmpz(fmpz_t res, apint_srcptr src)
{
    fmpz_init2(res, src->length);

    fmpz_set_ui(res, src->limbs[0]);
    for (int i = 1; i < src->length; ++i) {
        fmpz_t val;
        fmpz_set_ui(val, src->limbs[i]);
        fmpz_mul_2exp(val, val, sizeof(apint_limb_t) * 8 * i);
        fmpz_add(res, res, val);
    }
}

void apint_print(apint_srcptr value)
{
    fmpz_t number;
    apint_to_fmpz(number, value);

    fmpz_print(number);
    fmpz_clear(number);
}

void apint_free(apint_t x)
{
    free(x->limbs);

    x->length = 0;
    x->limbs = NULL;
}

void apint_copy(apint_ptr dst, apint_srcptr src)
{
    dst->length = src->length;
    dst->limbs = realloc(dst->limbs, src->length * APINT_LIMB_BYTES);
    memcpy(dst->limbs, src->limbs, src->length * APINT_LIMB_BYTES);
}

// right shift
void apint_shiftr(apint_ptr x, unsigned int shift)
{
    assert(x->limbs);

    if (!shift)
        return;

    size_t sl, sr;
    for (apint_size_t i = 0; i < (x->length - 1); i++)
    {
        sr = shift;
        sl = APINT_LIMB_BITS - shift;

        x->limbs[i] = (x->limbs[i + 1] << sl) | (x->limbs[i] >> sr);
    }

    x->limbs[x->length - 1] >>= shift;
}

void apint_shiftl(apint_ptr x, unsigned int shift){
    assert(x->limbs);
    if (shift == 0) return;

    uint full_limbs_shifted = shift / APINT_LIMB_BITS;
    shift -= full_limbs_shifted * APINT_LIMB_BITS;
    for (int i = x->length - 1; i >= 0; i--) {
        if (i-(int)full_limbs_shifted >= 0) {
            x->limbs[i] = x->limbs[i-full_limbs_shifted];
        }

        else {
            x->limbs[i] = 0;
        }
    }

    for (int i = x->length - 1; i > 0 ; i--) {
        x->limbs[i] = (x->limbs[i] << shift) + (x->limbs[i-1] >> (APINT_LIMB_BITS - shift));
    }
    x->limbs[0] <<= shift;
}

void apint_add(apint_ptr x, apint_srcptr a, apint_srcptr b)
{
    //sign_t borrow;
    if(a->sign==b->sign)
    {
        apint_plus(x,a,b);
        x->sign=a->sign;
    }
    else
    {
        if(a->sign==-1)//only a is negative. so equivalent to b-a.
        {
            x->sign=apint_minus(x,b,a);
        }
        else
        { //only b is negative.
            x->sign = apint_minus(x,a,b);
        }
    }
}

void apint_sub(apint_ptr x, apint_srcptr a, apint_srcptr b)
{
    if(a->sign==b->sign)
    {
        if (a->sign == 1) //both are positive
        {
            x->sign = apint_minus(x, a, b);
        }
        else //both are negative
        {
            x->sign = apint_minus(x, a, b);
        }
    }
    else
    {
        apint_plus(x,a,b);
        x->sign = a->sign;
    }
}

char apint_plus(apint_ptr x, apint_srcptr a, apint_srcptr b)
{
    assert(x->limbs && a->limbs && b->limbs);
    assert(a->length == b->length);
    assert(a->length == x->length);

    char carry = 0;

    for (apint_size_t i = 0; i < a->length; i++)
    {
        carry = _addcarryx_u64(carry, a->limbs[i], b->limbs[i], &x->limbs[i]);
    }
    return carry;
}

// |a| - |b|. Do not handle sign here.
sign_t apint_minus(apint_ptr x, apint_srcptr a, apint_srcptr b)
{
    assert(x->limbs && a->limbs && b->limbs);
    assert(a->length == b->length); // only handle same lengths for now
    assert(a->length == x->length);
    sign_t result_sign;
    unsigned char borrow = 0;

    if(apint_is_greater(a,b)) // a > b so a-b
    {
        result_sign=1;
        for (apint_size_t i = 0; i < a->length; i++)
        {
            borrow = _subborrow_u64(borrow, a->limbs[i], b->limbs[i], &x->limbs[i]);
        }
    }
    else // b > a so -(b-a)
    {
        result_sign=-1;
        for (apint_size_t i = 0; i < a->length; i++)
        {
            borrow = _subborrow_u64(borrow, b->limbs[i], a->limbs[i], &x->limbs[i]);
        }
    }
    return result_sign;
}


int apint_is_greater(apint_srcptr a, apint_srcptr b)
{
    //Works only for same length a, b
    //TODO: Check if there's a vector intrinsic for the comparison
    //TODO: Verify that the most significant limb understanding is correct
    for (apint_size_t i = 0; i < (a->length - 1); i++)
    {
        if(a->limbs[i] >b->limbs[i])
            return 1;
    }
    return 0;
}

void apint_mul(apint_ptr x, apint_srcptr a, apint_srcptr b)
{
    assert(x->limbs && a->limbs && b->limbs);
    assert(a->length == b->length); // only handle same lengths for now
    assert(a->length + b->length == x->length);

    unsigned long long overflow = 0;
    if(a->sign==b->sign)
        x->sign = 1;
    else
        x->sign =-1;

    for (apint_size_t i = 0; i < b->length; i++)
    {
        for (apint_size_t j = 0; j < a->length; j++)
        {
            x->limbs[i + j] += overflow;
            x->limbs[i + j] += _mulx_u64(a->limbs[j], b->limbs[i], &overflow);
        }
        overflow = 0;
    }
}

void apint_div(apint_ptr x, apint_srcptr a, apint_srcptr b)
{
    // To-do: Implement division.
}
