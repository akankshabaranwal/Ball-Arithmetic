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
    x->limbs = calloc(x->length, APINT_LIMB_BYTES);
    assert(x->limbs != NULL);
}

void apint_to_fmpz(fmpz_t res, apint_srcptr src)
{
    fmpz_init2(res, src->length);

    fmpz_set_ui(res, src->limbs[0]);
    for (int i = 1; i < src->length; ++i)
    {
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
    for (i = 0; i < src->length; i++)
    {
        dst->limbs[i] = src->limbs[i];
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
    for (i = x->length - 1; i >= 0; i--)
    {
        if (x->limbs[i] & UINT64_MAX) //means there's a 1 somewhere here
        {
            // Detect the position of first 1 here.
            number = x->limbs[i];
            while (number != 0)
            {
                if ((number & 0x01) != 0)
                {
                    pos++;
                    return pos;
                }
                number >>= 1;
            }
        }
        pos = pos + APINT_LIMB_BITS;
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
    for (int i = 0; i < x->length; ++i)
    {
        if (i + full_limbs_shifted < x->length)
        {
            x->limbs[i] = x->limbs[i + full_limbs_shifted];
            //printf("assign full limb here %d \n", full_limbs_shifted);
        }
        else
        {
            x->limbs[i] = 0;
        }
    }

    for (int i = 0; i < x->length - 1; ++i)
    {
        if (!shift)
            return;
        x->limbs[i] = (x->limbs[i] >> shift) + (x->limbs[i + 1] << (APINT_LIMB_BITS - shift));
    }

    x->limbs[x->length - 1] >>= shift;
}

void apint_shiftl(apint_ptr x, unsigned int shift)
{
    assert(x->limbs);
    if (shift == 0)
        return;

    uint full_limbs_shifted = shift / APINT_LIMB_BITS;
    shift -= full_limbs_shifted * APINT_LIMB_BITS;

    for (int i = x->length - 1; i >= 0; i--)
    {
        if (i - (int)full_limbs_shifted >= 0)
        {
            x->limbs[i] = x->limbs[i - full_limbs_shifted];
        }

        else
        {
            x->limbs[i] = 0;
        }
    }

    if (!shift)
        return;

    for (int i = x->length - 1; i > 0; i--)
    {
        x->limbs[i] = (x->limbs[i] << shift) + (x->limbs[i - 1] >> (APINT_LIMB_BITS - shift));
    }

    x->limbs[0] <<= shift;
}

unsigned char apint_add(apint_ptr x, apint_srcptr a, apint_srcptr b)
{
    unsigned char overflow;
    if (a->sign == b->sign)
    {
        overflow = apint_plus(x, a, b);
        x->sign = a->sign;
    }
    else
    {
        if (a->sign == -1) //only a is negative. so equivalent to b-a.
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
    if (a->sign == b->sign)
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
        x->limbs[i] = (unsigned)carry;
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

    if (apint_is_greater(a, b)) // a > b so a-b
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
        if (apint_getlimb(a, i) > apint_getlimb(b, i))
            return 1;
    }
    return 0;
}

int apint_mul(apint_ptr x, apint_srcptr a, apint_srcptr b)
{
    //TODO: check if these checks are needed anywhere else in the code.
    assert(x->limbs && a->limbs && b->limbs);
    assert(a->length == b->length); // only handle same lengths for now
    assert(a->length == x->length);

    unsigned long long overflow;
    if (a->sign == b->sign)
        x->sign = 1;
    else
        x->sign = -1;

    // Handling sign?

    for (apint_size_t i = 0; i < b->length; i++)
    {
        overflow = 0;
        for (apint_size_t j = 0; j < a->length; j++)
        {
            if ((i + j) < x->length) // make sure we don't try to set something in x that is outside of its precision
            {
                x->limbs[i + j] += overflow;
                x->limbs[i + j] += _mulx_u64(a->limbs[j], b->limbs[i], &overflow);
            }
        }
    }
    return (int)overflow;
}

void apint_mul_portable(apint_ptr x, apint_srcptr a, apint_srcptr b)
{
    assert(x->limbs && a->limbs && b->limbs);
    assert(a->length == b->length); // only handle same lengths for now
    assert(a->length + b->length <= x->length);

    if (a->sign == b->sign)
    {
        x->sign = 1;
    }
    else
    {
        x->sign = -1;
    }

    // Use 32 bits for multiplication to be able to get the overflow
    uint32_t overflow = 0;
    uint32_t *b_vals = (uint32_t *)b->limbs;
    uint32_t *a_vals = (uint32_t *)a->limbs;
    uint32_t *x_vals = (uint32_t *)x->limbs;
    for (size_t b_i = 0; b_i < b->length * 2; b_i++)
    {
        for (size_t a_i = 0; a_i < a->length * 2; a_i++)
        {
            x_vals[a_i + b_i] += overflow;
            uint64_t res = (uint64_t)a_vals[a_i] * (uint64_t)b_vals[b_i];
            overflow = res >> 32;
            x_vals[a_i + b_i] += res;
        }
        overflow = 0;
    }
}

void apint_div(apint_ptr x, apint_srcptr a, apint_srcptr b)
{
    // To-do: Implement division.

    // keep trying greatest powers of 2 and if it doesnt work shift, otherwise multiply (well its by 1 anyways), but just subtract
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

// // YOU can'T JUst SpeCIFY WhatEVEr THe fuCK PreciSion YOu WANt FOR THe OutpuT??????!!!!!!!!!
// // also we have to handle negatives, but I think that's more important in fp
// uint64_t apint_mul(apint_ptr x, apint_srcptr a, apint_srcptr b)
// { // Commented below because it was erroring out.
//     // To-do: Implement multiplication.
//     assert(x->limbs && a->limbs && b->limbs);
//     assert(a->length == b->length); // only handle same lengths for now
//     assert(a->length == x->length); // makes sense for the precision of the output to be the same as the input

//     // I don't think there would be an overflow for multiplication because
//     // the biggest possible number takes up bits equal to the sum of bits
//     // in a and b
//     uint64_t overflow = 0;

//     // Handling sign?

//     for (apint_size_t i = 0; i < b->length; i++)
//     {
//         for (apint_size_t j = 0; j < a->length; j++)
//         {
//             if ((i + j) < x->length) // I wasn't sure how else to make sure we don't update a limb in x that doesn't exist
//             {
//                 x->limbs[i + j] += overflow;
//                 x->limbs[i + j] += _mulx_u64(a->limbs[j], b->limbs[i], &overflow);
//             }
//         }
//         if ((i + a->length) < x->length)
//             x->limbs[i + a->length] += overflow;
//     }
//     return overflow; // This overflow doesn't actually mean anything because we give x enough space in the beginning anyways
// }

uint64_t apint_mul_karatsuba(apint_ptr x, apint_srcptr a, apint_srcptr b)
{
    assert(x->limbs && a->limbs && b->limbs);
    assert(a->length == b->length); // only handle same lengths of input
    assert(a->length == x->length); // assuming that output has same precision as both inputs

    // TODO: - Make sure to keep track of the sign, but pass everything in as positive
    //       - Does everything in Karatsuba break if we assume all positive?
    x->sign = a->sign * b->sign;

    // if a < 10 or b < 10, return a*b
    if (a->length <= 1 || b->length <= 1) // they have to be the same length anyways
    {
        // printf("Small enough size, returning\n");
        return apint_mul(x, a, b);
    }

    uint64_t overflow = apint_mul_karatsuba_recurse(x, a, b); // Although I don't think there will be overflow here
    return overflow;                                          // this returns a unit64_t
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

    // x_high, x_low = split x at d, or right shift by d
    apint_t a_high, a_low;
    a_high->sign = 1;
    a_low->sign = 1;
    apint_init(a_high, (a->length - d) * 64); // The 64 bits here is under the assumption that we use ints to represent everything
    apint_init(a_low, d * 64);

    apint_copyover(a_low, a, 0); // Pretty sure I can use the apint_limb function but that just sets one limb right?
    apint_copyover(a_high, a, d);

    // y_high, y_low = split y at d, or right shift by d
    apint_t b_high, b_low;
    b_high->sign = 1;
    b_low->sign = 1;
    apint_init(b_high, (b->length - d) * 64);
    apint_init(b_low, d * 64);

    apint_copyover(b_low, b, 0);
    apint_copyover(b_high, b, d);

    apint_t z0, z1, z2;
    // apint_init(z0, (a_high->length + b_high->length) * 64);
    // apint_init(z1, ((max(a_high->length, a_low->length)) + (max(b_high->length, b_low->length))) * 64);
    // apint_init(z2, (a_low->length + b_low->length) * 64);
    z0->sign = 1;
    z1->sign = 1;
    z2->sign = 1;
    apint_init(z0, x->length * 64); // Padding it to oblivion, trade off between performance and precision
    apint_init(z1, x->length * 64);
    apint_init(z2, x->length * 64);

    // printf("z0: 0x%llx 0x%llx 0x%llx 0x%llx\n", apint_getlimb(z0, 3), apint_getlimb(z0, 2), apint_getlimb(z0, 1), apint_getlimb(z0, 0));

    apint_t a_add, b_add;
    a_add->sign = 1;
    b_add->sign = 1;
    char a_add_overflow, b_add_overflow;
    apint_init(a_add, (max(a_high->length, a_low->length)) * 64);
    apint_init(b_add, (max(b_high->length, b_low->length)) * 64);

    a_add_overflow = apint_add_karatsuba(a_add, a_high, a_low); // a_high and a_low have to be the same length for now ASSUMPTION
    b_add_overflow = apint_add_karatsuba(b_add, b_high, b_low); // a_high and a_low have to be the same length for now

    apint_mul_karatsuba_recurse(z0, a_low, b_low);
    apint_mul_karatsuba_recurse(z1, a_add, b_add);   // THE LENGTH NEVER DECREASES, ok now it decreases, so its fine
    apint_mul_karatsuba_recurse(z2, a_high, b_high); // There should be an overflow but I don't think I need to do anything with it

    // printf("z0 after recursion: 0x%llx 0x%llx\n", apint_getlimb(z0, 1), apint_getlimb(z0, 0));
    // printf("z1 after recursion: 0x%llx 0x%llx\n", apint_getlimb(z1, 1), apint_getlimb(z1, 0));
    // printf("z2 after recursion: 0x%llx 0x%llx\n", apint_getlimb(z2, 1), apint_getlimb(z2, 0));

    // printf("z0 size after recursion: %d\n", z0->length);
    // printf("z1 size after recursion: %d\n", z1->length);
    // printf("z2 size after recursion: %d\n", z2->length);
    // apint_t z2_, z1_;
    // apint_init(z2_, (x->length) * 64);
    // apint_init(z1_, (x->length) * 64);

    apint_t first_operand; // z2 + z0
    first_operand->sign = 1;
    apint_init(first_operand, (z2->length) * 64);
    apint_add(first_operand, z2, z0);

    // printf("z2 + z0 = 0x%llx 0x%llx\n", apint_getlimb(first_operand, 1), apint_getlimb(first_operand, 0));

    apint_t second_operand; // z1 - (z2 + z0)
    second_operand->sign = 1;
    apint_init(second_operand, (z1->length) * 64);
    apint_sub(second_operand, z1, first_operand);

    // printf("z1 sign: %d\n", z1->sign);
    // printf("first_operand sign: %d\n", first_operand->sign);

    // printf("z1 = 0x%llx 0x%llx\n", apint_getlimb(z1, 1), apint_getlimb(z1, 0));
    // printf("z1 - (z2 + z0) = 0x%llx 0x%llx\n", apint_getlimb(second_operand, 1), apint_getlimb(second_operand, 0));

    // printf("z2 before shift: 0x%llx 0x%llx\n", apint_getlimb(z2, 1), apint_getlimb(z2, 0));
    // printf("z1 - (z2 + z0) before shift: 0x%llx 0x%llx\n", apint_getlimb(second_operand, 1), apint_getlimb(second_operand, 0));

    // Shift results appropriately, should be stored in z2_ and z1_
    printf("shifting by %d\n", d);
    d = d * 64;                // in the beginning we split by d, but d is a limb, which is 64 bits
    apint_shiftl(z2, (2 * d)); // multiply by 2 because of Karatsuba algorithm
    apint_shiftl(second_operand, d);

    // printf("z2 after shift: 0x%llx 0x%llx\n", apint_getlimb(z2, 1), apint_getlimb(z2, 0));
    // printf("z1 - (z2 + z0) after shift: 0x%llx 0x%llx\n", apint_getlimb(second_operand, 1), apint_getlimb(second_operand, 0));

    apint_t temp_x;
    apint_init(temp_x, (x->length) * 64);
    apint_add_karatsuba(temp_x, z2, second_operand);
    apint_add_karatsuba(x, temp_x, z0);
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
