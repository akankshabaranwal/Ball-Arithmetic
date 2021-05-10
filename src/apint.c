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

void apint_print(apint_srcptr value)
{
    fmpz_t number;
    fmpz_init2(number, value->length);

    fmpz_set_ui(number, value->limbs[0]);
    for (int i = 1; i < value->length; ++i)
    {
        fmpz_t val;
        fmpz_set_ui(val, value->limbs[i]);
        fmpz_mul_2exp(val, val, sizeof(apint_limb_t) * 8 * i);
        fmpz_add(number, number, val);
    }

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

char apint_add(apint_ptr x, apint_srcptr a, apint_srcptr b)
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

// a - b
void apint_sub(apint_ptr x, apint_srcptr a, apint_srcptr b)
{
    // To-do: Implement substraction. Possibly add a negative value flag in apint_t.
    assert(x->limbs && a->limbs && b->limbs);
    assert(a->length == b->length); // only handle same lengths for now
    assert(a->length == x->length);

    char borrow = 0;

    for (apint_size_t i = 0; i < a->length; i++)
    {
        borrow = _subborrow_u64(borrow, a->limbs[i], b->limbs[i], &x->limbs[i]);
    }
}

uint64_t apint_mul(apint_ptr x, apint_srcptr a, apint_srcptr b)
{ // Commented below because it was erroring out.
    // To-do: Implement multiplication.
    assert(x->limbs && a->limbs && b->limbs);
    assert(a->length == b->length); // only handle same lengths for now
    assert(a->length + b->length == x->length);

    uint64_t overflow = 0;

    // Need to check if a or b is smaller to make more efficient

    for (apint_size_t i = 0; i < b->length; i++)
    {
        for (apint_size_t j = 0; j < a->length; j++)
        {
            x->limbs[i + j] += overflow;
            x->limbs[i + j] += _mulx_u64(a->limbs[j], b->limbs[i], &overflow);
        }
        x->limbs[i + a->length] += overflow;
    }
    return overflow;
}

/*
This is a recursive method
*/
void apint_mul_karatsuba(apint_ptr x, apint_srcptr a, apint_srcptr b)
{
    assert(x->limbs && a->limbs && b->limbs);
    assert(a->length == b->length);             // only handle same lengths for now
    assert(a->length + b->length == x->length); // store enough space for x to handle maximum number of bits

    // x->limbs = apint_mul_karatsuba_recurse(x, a, b);
}

void apint_mul_karatsuba_recurse(apint_ptr x, apint_srcptr a, apint_srcptr b)
{
}

void apint_div(apint_ptr x, apint_srcptr a, apint_srcptr b)
{
    // To-do: Implement division.
}
