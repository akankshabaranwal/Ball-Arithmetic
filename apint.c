#include <assert.h>
#include <immintrin.h>

#include "apint.h"

void apint_add(apint_t *dst, apint_t *v1, apint_t *v2)
{
    // To-do: Support adding AP ints with differing lengths.
    assert(v1->length == v2->length);

    dst->length = v1->length;

    char carry = 0;
    for (size_t i = 0; i < v1->length; i++)
    {
        carry = _addcarryx_u64(carry, v1->values[i], v2->values[i], &dst->values[i]);
    }
}

void apint_sub(apint_t *dst, apint_t *v1, apint_t *v2)
{
    // To-do: Implement substraction. Possibly add a negative value flag in apint_t.
}

void apint_mul(apint_t *dst, apint_t *v1, apint_t *v2)
{
    // To-do: Implement multiplication.
}

void apint_div(apint_t *dst, apint_t *v1, apint_t *v2)
{
    // To-do: Implement division.
}
