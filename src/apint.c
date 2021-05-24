#include <assert.h>
#include <x86intrin.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <math.h>

#include <apint.h>
#include <flint/fmpz.h>

void apint_init(apint_t x, apint_size_t p)
{
    x->length = (p / APINT_LIMB_BITS) + ((p % APINT_LIMB_BITS) > 0);
    x->limbs = malloc(x->length * APINT_LIMB_BYTES);

    /*
    JULIA: Is this below worth adding? 
    just in case we don't end up changing one of the limbs in our value 
    so we want to start off with initializing everything to 0x0?
    */
    for (int i = 0; i < x->length; i++)
        apint_setlimb(x, i, 0);
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
char apint_sub(apint_ptr x, apint_srcptr a, apint_srcptr b)
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

    return borrow;
}

// YOU can'T JUst SpeCIFY WhatEVEr THe fuCK PreciSion YOu WANt FOR THe OutpuT??????!!!!!!!!!
uint64_t apint_mul(apint_ptr x, apint_srcptr a, apint_srcptr b)
{ // Commented below because it was erroring out.
    // To-do: Implement multiplication.
    assert(x->limbs && a->limbs && b->limbs);
    assert(a->length == b->length); // only handle same lengths for now
    assert(a->length == x->length); // makes sense for the precision of the output to be the same as the input

    // I don't think there would be an overflow for multiplication because
    // the biggest possible number takes up bits equal to the sum of bits
    // in a and b
    uint64_t overflow = 0;

    // Handling sign?

    for (apint_size_t i = 0; i < b->length; i++)
    {
        for (apint_size_t j = 0; j < a->length; j++)
        {
            if ((i + j) < x->length) // I wasn't sure how else to make sure we don't update a limb in x that doesn't exist
            {
                x->limbs[i + j] += overflow;
                x->limbs[i + j] += _mulx_u64(a->limbs[j], b->limbs[i], &overflow);
            }
        }
        if ((i + a->length) < x->length)
            x->limbs[i + a->length] += overflow;
    }
    return overflow; // This overflow doesn't actually mean anything then if we drop the higher bits of the result anyways
}

uint64_t apint_mul_karatsuba(apint_ptr x, apint_srcptr a, apint_srcptr b)
{
    // printf("Length a: %d\n", a->length);
    // printf("Length b: %d\n", b->length);
    // printf("Length x: %d\n", x->length);

    assert(x->limbs && a->limbs && b->limbs);
    assert(a->length == b->length); // only handle same lengths of input
    assert(a->length == x->length); // assuming that output has same precision as both inputs

    // if a < 10 or b < 10, return a*b
    if (a->length <= 1 || b->length <= 1)
    {
        // printf("Small enough size, returning\n");
        return apint_mul_karatsuba_base_case(x, a, b);
    }

    printf("b before: 0x%llx 0x%llx\n", apint_getlimb(b, 1), apint_getlimb(b, 0));

    // int x_original_length = x->length; // this was for some reason overwriting b?
    apint_t x_new;
    apint_init(x_new, (a->length + b->length) * 64); // making x longer for now...

    printf("b after: 0x%llx 0x%llx\n", apint_getlimb(b, 1), apint_getlimb(b, 0));

    uint64_t overflow = apint_mul_karatsuba_recurse(x_new, a, b); // Although I don't think there will be overflow here

    // need to bring x down to the original number of precision
    apint_trim(x, x_new); // bring x_new back down to precision of x

    apint_free(x_new);
    return overflow; // this returns a unit64_t
}

/*
This is a recursive method
*/
uint64_t apint_mul_karatsuba_recurse(apint_ptr x, apint_srcptr a, apint_srcptr b)
{
    // if a < 10 or b < 10, return a*b
    if (a->length == 1 || b->length == 1)
    {
        printf("Small enough size, returning\n");
        return apint_mul_karatsuba_base_case(x, a, b);
    }

    // d = floor(max(length(a), length(b)) / 2)
    apint_size_t d = floor(max(a->length, b->length) / 2); // They're the same length anyways

    // okay at some point a length is 0?

    // x_high, x_low = split x at d, or right shift by d
    apint_t a_high, a_low;
    apint_init(a_high, (a->length - d) * 64); // The 64 bits here is under the assumption that we use ints to represent everything
    apint_init(a_low, d * 64);

    apint_copyover(a_low, a, 0); // Pretty sure I can use the apint_limb function but that just sets one limb right?
    apint_copyover(a_high, a, d);

    // y_high, y_low = split y at d, or right shift by d
    apint_t b_high, b_low;
    apint_init(b_high, (b->length - d) * 64);
    apint_init(b_low, d * 64);

    apint_copyover(b_low, b, 0);
    apint_copyover(b_high, b, d);

    apint_t z0, z1, z2;
    apint_init(z0, (a_high->length + b_high->length) * 64);
    apint_init(z1, ((max(a_high->length, a_low->length)) + (max(b_high->length, b_low->length))) * 64);
    apint_init(z2, (a_low->length + b_low->length) * 64);
    // apint_init(z0, (a->length + b->length) * 64); // Padding it to oblivion, trade off between performance and precision
    // apint_init(z1, (a->length + b->length) * 64);
    // apint_init(z2, (a->length + b->length) * 64);

    // printf("z0: 0x%llx 0x%llx 0x%llx 0x%llx\n", apint_getlimb(z0, 3), apint_getlimb(z0, 2), apint_getlimb(z0, 1), apint_getlimb(z0, 0));

    apint_t a_add, b_add;
    char a_add_overflow, b_add_overflow;
    apint_init(a_add, (max(a_high->length, a_low->length)) * 64);
    apint_init(b_add, (max(b_high->length, b_low->length)) * 64);

    a_add_overflow = apint_add_karatsuba(a_add, a_high, a_low); // a_high and a_low have to be the same length for now ASSUMPTION
    b_add_overflow = apint_add_karatsuba(b_add, b_high, b_low); // a_high and a_low have to be the same length for now

    // printf("Length a_add: %d\n", a_add->length);
    // printf("Length b_add: %d\n", b_add->length);
    // printf("Length z1: %d\n", z1->length);
    // printf("a_add after adding: 0x%llx 0x%llx\n", apint_getlimb(a_add, 1), apint_getlimb(a_add, 0));
    // printf("b_add after adding: 0x%llx 0x%llx\n", apint_getlimb(b_add, 1), apint_getlimb(b_add, 0));

    // printf("Length z0: %d\n", z0->length);
    // printf("Length z1: %d\n", z1->length);
    // printf("Length z2: %d\n", z2->length);
    // printf("Length a_high: %d\n", a_high->length);
    // printf("Length b_high: %d\n", b_high->length);
    // printf("Length a_add: %d\n", a_add->length);
    // printf("Length b_add: %d\n", b_add->length);
    // printf("Length a_low: %d\n", a_low->length);
    // printf("Length b_low: %d\n\n", b_low->length);

    apint_mul_karatsuba_recurse(z0, a_high, b_high); // There should be an overflow but I don't think I need to do anything with it
    apint_mul_karatsuba_recurse(z1, a_add, b_add);   // THE LENGTH NEVER DECREASES, ok now it decreases, so its fine
    apint_mul_karatsuba_recurse(z2, a_low, b_low);

    printf("Length z0: %d\n", z0->length);
    printf("Length z1: %d\n", z1->length);
    printf("Length z2: %d\n", z2->length);
    printf("LENGTH OF x: %d\n", x->length);

    printf("z0 before shift: 0x%llx 0x%llx\n", apint_getlimb(z0, 1), apint_getlimb(z0, 0));
    printf("z1 before shift: 0x%llx 0x%llx\n", apint_getlimb(z1, 1), apint_getlimb(z1, 0));
    printf("z2 before shift: 0x%llx 0x%llx\n", apint_getlimb(z2, 1), apint_getlimb(z2, 0));

    apint_t z2_, z1_;
    apint_init(z2_, (x->length) * 64);
    apint_init(z1_, (x->length) * 64);

    apint_t z1_z2, z1_z2_z0;
    apint_init(z1_z2, (z1->length) * 64); // should be same length as operands
    apint_init(z1_z2_z0, (z1->length) * 64);
    apint_sub(z1_z2, z1, z2);
    apint_sub(z1_z2_z0, z1_z2, z0);

    printf("z2_ before shift: 0x%llx 0x%llx\n", apint_getlimb(z2, 1), apint_getlimb(z2, 0));
    printf("z1_ before shift: 0x%llx 0x%llx\n", apint_getlimb(z1_z2_z0, 1), apint_getlimb(z1_z2_z0, 0));

    apint_shiftl_by_d(z2_, z2, 2 * d);
    apint_shiftl_by_d(z1_, z1_z2_z0, d);

    printf("z2_ before shift: 0x%llx 0x%llx\n", apint_getlimb(z2_, 1), apint_getlimb(z2_, 0));
    printf("z1_ before shift: 0x%llx 0x%llx\n", apint_getlimb(z1_, 1), apint_getlimb(z1_, 0));

    apint_t x_;
    apint_init(x_, (x->length) * 64);
    apint_add_karatsuba(x_, z2_, z1_);
    apint_add_karatsuba(x, x_, z0);
    // x = z2 * 2 ^ (2 * d) + (z1 - z2 - z0) * 2 ^ (d) + z0;

    uint64_t result; // not used yet
    return result;
}

// YOU can'T JUst SpeCIFY WhatEVEr THe fuCK PreciSion YOu WANt FOR THe OutpuT??????!!!!!!!!!
uint64_t apint_mul_karatsuba_base_case(apint_ptr x, apint_srcptr a, apint_srcptr b)
{
    assert(x->limbs && a->limbs && b->limbs);
    assert(a->length == b->length);               // only handle same lengths for now
    assert((a->length + b->length) == x->length); // output can be different precision than input due to recursion and not wanting to lose precision during recursion

    // I don't think there would be an overflow for multiplication because the biggest possible number takes up bits equal to the sum of bits in a and b
    uint64_t overflow = 0;

    // Handling sign?

    for (apint_size_t i = 0; i < b->length; i++)
    {
        for (apint_size_t j = 0; j < a->length; j++)
        {
            if ((i + j) < x->length) // make sure we don't try to set something in x that is outside of its precision
            {
                x->limbs[i + j] += overflow;
                x->limbs[i + j] += _mulx_u64(a->limbs[j], b->limbs[i], &overflow);
            }
        }
        if ((i + a->length) < x->length)
            x->limbs[i + a->length] += overflow;
    }
    return overflow; // This overflow doesn't actually mean anything then if we drop the higher bits of the result anyways
}

char apint_add_karatsuba(apint_ptr x, apint_srcptr a, apint_srcptr b)
{
    assert(x->limbs && a->limbs && b->limbs);
    assert((max(a->length, b->length)) == x->length);
    /*
    a has to be the bigger one because when we pass in a_high,
    it will always be greater or equal because of how we calculate "d"
    */
    printf("add_karatsuba a length: %d\n", a->length);
    printf("add_karatsuba b length: %d\n", b->length);
    assert(a->length >= b->length);

    char carry = 0;
    apint_size_t i;
    for (i = 0; i < b->length; i++)
    {
        carry = _addcarryx_u64(carry, a->limbs[i], b->limbs[i], &x->limbs[i]);
    }

    for (; i < a->length; i++)
    {
        carry = _addcarryx_u64(carry, a->limbs[i], 0, &x->limbs[i]);
    }

    // x->limbs[x->length - 1] = carry;
    return carry;
}

void apint_div(apint_ptr x, apint_srcptr a, apint_srcptr b)
{
    // To-do: Implement division.

    // keep trying greatest powers of 2 and if it doesnt work shift, otherwise multiply (well its by 1 anyways), but just subtract
}
