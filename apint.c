#include <assert.h>
#include <immintrin.h>
#include <stdlib.h>

#include "apint.h"

void apint_init(apint_t x, apint_size_t p)
{
    x->length = (p / APINT_LIMB_BITS) + ((p % APINT_LIMB_BITS) > 0);
    x->limbs = malloc(x->length * APINT_LIMB_BYTES);
}

void apint_free(apint_t x)
{
    free(x->limbs);

    x->length = 0;
    x->limbs = NULL;
}

void apint_shiftr(apint_ptr x, size_t shift)
{
    assert(x->limbs);

    size_t sl, sr;
    for (apint_size_t i = 0; i < (x->length - 1); i++)
    {
        sr = shift;
        sl = (sizeof(apint_limb_t) * 8) - shift;

        x->limbs[i] = (x->limbs[i + 1] << sl) | (x->limbs[i] >> sr);
    }

    x->limbs[x->length - 1] >>= shift;
}

void apint_add(apint_ptr x, apint_srcptr a, apint_srcptr b)
{
    assert(x->limbs && a->limbs && b->limbs);
    assert(a->length == b->length);
    assert(a->length == x->length);

    char carry = 0;

    for (apint_size_t i = 0; i < a->length; i++)
    {
        carry = _addcarryx_u64(carry, a->limbs[i], b->limbs[i], &x->limbs[i]);
    }
}

void apint_sub(apint_ptr x, apint_srcptr a, apint_srcptr b)
{
    // To-do: Implement substraction. Possibly add a negative value flag in apint_t.
}

void apint_mul(apint_ptr x, apint_srcptr a, apint_srcptr b)
{
    // To-do: Implement multiplication.
}

void apint_div(apint_ptr x, apint_srcptr a, apint_srcptr b)
{
    // To-do: Implement division.
}
