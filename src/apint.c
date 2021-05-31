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
    x->limbs = calloc(x->length,  APINT_LIMB_BYTES);
    assert(x->limbs != NULL);
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
    assert(dst->length >= src->length);
    /*
    dst->length = src->length;
    dst->limbs = realloc(dst->limbs, src->length * APINT_LIMB_BYTES);
    memcpy(dst->limbs, src->limbs, src->length * APINT_LIMB_BYTES);
    */
    int i;
    for(i =0;i<src->length;i++)
    {
        dst->limbs[i]=src->limbs[i];
    }
}

// detect the position of first 1
// naive method
int apint_detectfirst1(apint_ptr x)
{
    //Iterate over the limbs
    int i;
    int pos;
    apint_limb_t number;
    pos = 0;
    for(i = x->length-1; i>=0;i--)
    {
        if(x->limbs[i]&UINT64_MAX)//means there's a 1 somewhere here
        {
            // Detect the position of first 1 here.
            number = x->limbs[i];
            while(number != 0){
                if ((number & 0x01) != 0) {
                    pos++;
                    return pos;
                }
                number >>=1;
            }
        }
        pos = pos+APINT_LIMB_BITS;
    }
    return pos;
}

// right shift
void apint_shiftr(apint_ptr x, unsigned int shift)
{
    assert(x->limbs);

    if (!shift)
        return;

    uint full_limbs_shifted = shift / APINT_LIMB_BITS;
    shift -= full_limbs_shifted * APINT_LIMB_BITS;

    //printf("shift is %d \n", shift);
    for (int i = 0; i < x->length; ++i) {
        if (i + full_limbs_shifted < x->length) {
            x->limbs[i] = x->limbs[i+full_limbs_shifted];
            //printf("assign full limb here %d \n", full_limbs_shifted);
        }
        else {
            x->limbs[i] = 0;
        }
    }

    for (int i = 0; i < x->length - 1; ++i) {
    if (!shift)
        return;
        x->limbs[i] = (x->limbs[i] >> shift) + (x->limbs[i+1] << (APINT_LIMB_BITS - shift));
    }

    x->limbs[x->length-1] >>= shift;

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

    if (!shift)
        return;

    for (int i = x->length - 1; i > 0 ; i--) {
        x->limbs[i] = (x->limbs[i] << shift) + (x->limbs[i-1] >> (APINT_LIMB_BITS - shift));
    }

    x->limbs[0] <<= shift;
}

unsigned char apint_add(apint_ptr x, apint_srcptr a, apint_srcptr b)
{
    unsigned char overflow;
    if(a->sign == b->sign)
    {
        overflow = apint_plus(x,a,b);
        x->sign = a->sign;
    }
    else
    {
        if(a->sign == -1)//only a is negative. so equivalent to b-a.
        {
            overflow = apint_minus(x, b, a);
        }
        else
        { //only b is negative.
            overflow = apint_minus(x, a, b);
        }
    }
    return overflow;
}

unsigned char apint_sub(apint_ptr x, apint_srcptr a, apint_srcptr b)
{
    unsigned char overflow;
    if(a->sign == b->sign)
    {
        overflow = apint_minus(x, a, b); //sign is set here
    }
    else
    {
        apint_plus(x, a, b);
        x->sign = a->sign;
    }
    return overflow;
}

char apint_plus_portable(apint_ptr x, apint_srcptr a, apint_srcptr b)
{
    assert(x->limbs && a->limbs && b->limbs);
    assert(a->length == b->length);
    assert(a->length <= x->length);

    char carry = 0;

    for (apint_size_t i = 0; i < a->length; i++)
    {
        x->limbs[i] = (unsigned) carry;
        x->limbs[i] += a->limbs[i] + b->limbs[i];
        carry = (a->limbs[i] > UINT64_MAX - b->limbs[i]) ? 1 : 0;
    }
    return carry;
}

unsigned char apint_plus(apint_ptr x, apint_srcptr a, apint_srcptr b)
{
    assert(x->limbs && a->limbs && b->limbs);
    assert(a->length == b->length);
    assert(a->length <= x->length);

    unsigned char carry = 0;

    for (apint_size_t i = 0; i < a->length; i++)
    {
        carry = _addcarryx_u64(carry, a->limbs[i], b->limbs[i], &x->limbs[i]);
    }
    return carry;
}

// |a| - |b|. Do not handle sign here.
unsigned char apint_minus(apint_ptr x, apint_srcptr a, apint_srcptr b)
{
    assert(x->limbs && a->limbs && b->limbs);
    assert(a->length == b->length); // only handle same lengths for now
    assert(a->length <= x->length);
    unsigned char borrow = 0;

    if(apint_is_greater(a, b)) // a > b so a-b
    {
        x->sign = a->sign;
        for (apint_size_t i = 0; i < a->length; i++)
        {
            borrow = _subborrow_u64(borrow, a->limbs[i], b->limbs[i], &x->limbs[i]);
        }
    }
    else // b > a so -(b-a)
    {
        x->sign = -b->sign;
        for (apint_size_t i = 0; i < a->length; i++)
        {
            borrow = _subborrow_u64(borrow, b->limbs[i], a->limbs[i], &x->limbs[i]);
        }
    }
    return borrow;
}

int apint_is_greater(apint_srcptr a, apint_srcptr b)
{
    //Works only for same length a, b
    for (int i = (a->length - 1); i >= 0; i--)
    {
        if(apint_getlimb(a,i) > apint_getlimb(b,i))
            return 1;
    }
    return 0;
}

int apint_mul(apint_ptr x, apint_srcptr a, apint_srcptr b)
{
    //TODO: check if these checks are needed anywhere else in the code.
    assert(x->limbs && a->limbs && b->limbs);
    assert(a->length == b->length); // only handle same lengths for now
    assert(a->length + b->length <= x->length);

    unsigned long long overflow;
    if(a->sign == b->sign)
        x->sign = 1;
    else
        x->sign = -1;

    for (apint_size_t i = 0; i < b->length; i++)
    {
        overflow = 0;
        for (apint_size_t j = 0; j < a->length; j++)
        {
            x->limbs[i + j] += overflow;
            x->limbs[i + j] += _mulx_u64(a->limbs[j], b->limbs[i], &overflow);
        }
    }
    return (int) overflow;
}

void apint_mul_portable(apint_ptr x, apint_srcptr a, apint_srcptr b)
{
    assert(x->limbs && a->limbs && b->limbs);
    assert(a->length == b->length); // only handle same lengths for now
    assert(a->length + b->length <= x->length);

    if(a->sign == b->sign) {
        x->sign = 1;
    }
    else {
        x->sign = -1;
    }

    // Use 32 bits for multiplication to be able to get the overflow
    uint32_t overflow = 0;
    uint32_t *b_vals = (uint32_t *) b->limbs;
    uint32_t *a_vals = (uint32_t *) a->limbs;
    uint32_t *x_vals = (uint32_t *) x->limbs;
    for (size_t b_i = 0; b_i < b->length * 2; b_i++)
    {
        for (size_t a_i = 0; a_i < a->length * 2; a_i++)
        {
            x_vals[a_i + b_i] += overflow;
            uint64_t res = (uint64_t) a_vals[a_i] * (uint64_t) b_vals[b_i];
            overflow = res >> 32;
            x_vals[a_i + b_i] += res;
        }
        overflow = 0;
    }
}

void apint_div(apint_ptr x, apint_srcptr a, apint_srcptr b)
{
    // To-do: Implement division.
}
