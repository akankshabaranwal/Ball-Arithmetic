#include <assert.h>
#include <x86intrin.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <math.h>

#include <apint.h>
#include <flint/fmpz.h>

#define APINT_MSB ((apint_limb_t)1 << (sizeof(apint_limb_t) * 8 - 1))

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

    if (value->sign < 0) {
        printf("-");
    }
    fmpz_print(number);
    fmpz_clear(number);
}

void apint_print_msg(const char *msg, apint_srcptr value)
{
    printf("%s ", msg);
    apint_print(value);
    printf("\n");
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
    int i;
    for (i = 0; i < src->length; i+=4)
    {
        dst->limbs[i] = src->limbs[i];
        dst->limbs[i+1] = src->limbs[i+1];
        dst->limbs[i+2] = src->limbs[i+2];
        dst->limbs[i+3] = src->limbs[i+3];
    }
    dst->sign = src->sign;
}

// detect the position of first 1
// naive method
size_t apint_detectfirst1(apint_ptr x)
{
    //Iterate over the limbs
    size_t i;
    size_t pos;
    apint_limb_t number;
    pos = 0;
    for(i = x->length - 1; i >= 0; i--) {
        if(x->limbs[i] > 0) {
            // There's a 1 somewhere here
            number = x->limbs[i];
            while(1) {
                if (number & APINT_MSB) {
                    return x->length * APINT_LIMB_BITS - pos;
                }
                number <<= 1;
                pos++;
            }
        }
        pos += APINT_LIMB_BITS;
    }
    return x->length * APINT_LIMB_BITS - pos;
}
// Optimization 1
size_t apint_detectfirst1_optim1(apint_ptr x)
{
    //Iterate over the limbs
    size_t i;
    size_t pos;
    apint_limb_t number;
    pos = 0;
    for(i = x->length - 1; i >= 0; i--)
    {
        if(x->limbs[i] > 0)
        {
            number = x->limbs[i];
            break;
        }
    }
    pos = APINT_LIMB_BITS * (x->length-i-1);
    int xsize = x->length * APINT_LIMB_BITS;
    int bitpos = __builtin_clzll(number);
    pos = pos + bitpos;
    return (xsize-pos);
}

bool apint_shiftr_copy(apint_ptr dest, apint_srcptr src, unsigned int shift)
{
    assert(src->limbs);
    assert(dest->limbs);
    assert(dest->length >= src->length);

    if (!shift) return false;

    uint full_limbs_shifted = shift / APINT_LIMB_BITS;
    shift -= full_limbs_shifted * APINT_LIMB_BITS;

    bool did_shift = false;

    for (int i = 0; i < src->length; ++i) {
        if (i + full_limbs_shifted < src->length) {
            if (i == 0) {
                for (int j = 0; j < full_limbs_shifted; ++j) {
                    if (src->limbs[j] != 0) did_shift = true;
                }
            }
            dest->limbs[i] = (src->limbs[i+full_limbs_shifted] >> shift) + (src->limbs[i+full_limbs_shifted+1] << (APINT_LIMB_BITS - shift));
        }
        else {
            dest->limbs[i] = 0;
        }
    }

    dest->limbs[src->length - 1] = src->limbs[src->length-1] >> shift;
    return did_shift || __builtin_ctzl(src->limbs[0]) >= shift;
}

//First optimization. Removed branching. Reorganized function calls.
bool apint_shiftr(apint_ptr x, unsigned int shift)
{
    assert(x->limbs);

    if (!shift) return false;

    uint full_limbs_shifted = shift / APINT_LIMB_BITS;
    shift -= full_limbs_shifted * APINT_LIMB_BITS;

    bool did_shift = false;

    for (int i = 0; i < x->length; ++i) {
        if (i + full_limbs_shifted < x->length) {
            if (i == 0) {
                for (int j = 0; j < full_limbs_shifted; ++j) {
                    if (x->limbs[j] != 0) did_shift = true;
                }
            }
            x->limbs[i] = x->limbs[i+full_limbs_shifted];
        }
        else {
            x->limbs[i] = 0;
        }
    }

    if (!shift) return did_shift;
    did_shift |= __builtin_ctzl(x->limbs[0]) >= shift;

    for (int i = 0; i < x->length - 1; ++i) {
        x->limbs[i] = (x->limbs[i] >> shift) + (x->limbs[i+1] << (APINT_LIMB_BITS - shift));
    }

    x->limbs[x->length - 1] >>= shift;

    return did_shift;
}

/* Optimizations:
 * Unrolling by 2
 * Reorganized if else to remove branching
 * Not included: Mid point representation
*/
bool apint_shiftr_optim1(apint_ptr x, unsigned int shift)
{
    assert(x->limbs);

    if (!shift) return false;

    int full_limbs_shifted = shift / APINT_LIMB_BITS;
    shift -= full_limbs_shifted * APINT_LIMB_BITS;

    bool did_shift = false;

    int full_limbs_shifted_1 = full_limbs_shifted-1;
    for (int i = full_limbs_shifted; i  < x->length; i+=2)
    {
        x->limbs[i-full_limbs_shifted] = x->limbs[i];
        x->limbs[i-full_limbs_shifted_1] = x->limbs[i+1];
    }

    if (!shift) return did_shift;
    did_shift |= __builtin_ctzl(x->limbs[0]) >= shift;

    int leftshiftamt =  (APINT_LIMB_BITS - shift);
    for (int i = 0; i < x->length - 1; ++i)
    {
        x->limbs[i] = (x->limbs[i] >> shift) + (x->limbs[i+1] << leftshiftamt);
    }

    x->limbs[x->length - 1] >>= shift;
    return did_shift;
}

void apint_shiftl(apint_ptr x, unsigned int shift){
    assert(x->limbs);
    if (shift == 0) return;

    uint full_limbs_shifted = shift / APINT_LIMB_BITS;
    shift -= full_limbs_shifted * APINT_LIMB_BITS;

    for (int i = x->length - 1; i >= 0; i--) {
        if (i - (int) full_limbs_shifted >= 0) {
            x->limbs[i] = x->limbs[i-full_limbs_shifted];
        }
        else {
            x->limbs[i] = 0;
        }
    }

    if (!shift)
        return;

    for (int i = x->length - 1; i > 0; i--) {
        x->limbs[i] = (x->limbs[i] << shift) + (x->limbs[i-1] >> (APINT_LIMB_BITS - shift));
    }

    x->limbs[0] <<= shift;
}
/* Optimizations:
 * Refer _optim2 for the most optimized version
 */
void apint_shiftl_optim1(apint_ptr x, unsigned int shift){
    assert(x->limbs);
    if (shift == 0) return;

    int full_limbs_shifted = shift / APINT_LIMB_BITS;
    shift -= full_limbs_shifted * APINT_LIMB_BITS;

    for (int i = x->length - 1; i >= full_limbs_shifted; i--) {
            x->limbs[i] = x->limbs[i-full_limbs_shifted];
    }

    if (!shift)
        return;

    for (int i = x->length - 1; i > 0; i--) {
        x->limbs[i] = (x->limbs[i] << shift) + (x->limbs[i-1] >> (APINT_LIMB_BITS - shift));
    }

    x->limbs[0] <<= shift;
}

/* Optimizations:
 * Loop unrolling by a factor of 2
 * Type casting to same data type
 * Reorganized code to remove unnecessary branching
 * Removed unnecessary loop iterations
 * Scalar replacement
*/
void apint_shiftl_optim2(apint_ptr x, unsigned int shift){
    assert(x->limbs);
    if (shift == 0) return;

    int full_limbs_shifted = shift / APINT_LIMB_BITS;
    shift -= full_limbs_shifted * APINT_LIMB_BITS;
    int full_limbs_shifted_1 = full_limbs_shifted + 1;
    for (int i = x->length - 1; i >= full_limbs_shifted-1; i-=2) {
        x->limbs[i] = x->limbs[i-full_limbs_shifted];
        x->limbs[i-1] = x->limbs[i-full_limbs_shifted_1];
    }

    if (!shift)
        return;

    int rightshiftamt = (APINT_LIMB_BITS - shift);
    for (int i = x->length - 1; i > 0; i--) {
        x->limbs[i] = (x->limbs[i] << shift) + (x->limbs[i-1] >> rightshiftamt);
    }
    x->limbs[0] <<= shift;
}

// portable code
unsigned char apint_add_portable(apint_ptr x, apint_srcptr a, apint_srcptr b)
{
    unsigned char overflow;
    if (a->sign == b->sign)
    {
        overflow = apint_plus_portable(x, a, b);
        x->sign = a->sign;
    }
    else
    {
        if (a->sign == -1) //only a is negative. so equivalent to b-a.
        {
            overflow = apint_minus_portable(x, b, a);
        }
        else
        {
            overflow = apint_minus_portable(x, a, b);
        }
    }
    return overflow;
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
        {
            overflow = apint_minus(x, a, b);
        }
    }
    return overflow;
}

unsigned char apint_add_plus(apint_ptr x, apint_srcptr a, apint_srcptr b)
{
    unsigned char overflow;
    if (a->sign == b->sign)
    {
        overflow = apint_plus_optim1(x, a, b);
        x->sign = a->sign;
    }
    else
    {
        if (a->sign == -1) //only a is negative. so equivalent to b-a.
        {
            overflow = apint_minus_optim1(x, b, a);
        }
        else
        {
            overflow = apint_minus_optim1(x, a, b);
        }
    }
    return overflow;
}

unsigned char apint_sub_portable(apint_ptr x, apint_srcptr a, apint_srcptr b)
{
    unsigned char overflow;

    if (a->sign == b->sign)
    {
        overflow = apint_minus_portable(x, a, b); //sign is set here
    }
    else
    {
        apint_plus_portable(x, a, b);
        x->sign = a->sign;
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

unsigned char apint_sub_minus(apint_ptr x, apint_srcptr a, apint_srcptr b)
{
    unsigned char overflow;

    if (a->sign == b->sign)
    {
        overflow = apint_minus_optim1(x, a, b); //sign is set here
    }
    else
    {
        apint_plus_optim1(x, a, b);
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
/* Optimizations compared to portable version
 * Changed maximum iteration to midpoint
 * Using vector intrinsics
 * Unrolled by a factor of 4.
 */
unsigned char apint_plus_optim1(apint_ptr x, apint_srcptr a, apint_srcptr b)
{
    assert(x->limbs && a->limbs && b->limbs);
    assert(a->length == b->length);
    assert(a->length <= x->length);

    int midpt = (a->length /2) + 1;
    unsigned char carry1 = 0;
    unsigned char carry2 = 0;
    unsigned char carry3 = 0;
    unsigned char carry4 = 0;
    for (apint_size_t i = 0; i < midpt; i+=4)
    {
        carry1 = _addcarryx_u64(carry4, a->limbs[i], b->limbs[i], &x->limbs[i]);
        carry2 = _addcarryx_u64(carry1, a->limbs[i], b->limbs[i], &x->limbs[i]);
        carry3 = _addcarryx_u64(carry2, a->limbs[i], b->limbs[i], &x->limbs[i]);
        carry4 = _addcarryx_u64(carry3, a->limbs[i], b->limbs[i], &x->limbs[i]);
    }
    return carry4;
}

// |a| - |b|. Do not handle sign here.
unsigned char apint_minus_portable(apint_ptr x, apint_srcptr a, apint_srcptr b)
{
    assert(x->limbs && a->limbs && b->limbs);
    assert(a->length == b->length); // only handle same lengths for now
    assert(a->length <= x->length);
    unsigned char borrow = 0;
    apint_srcptr first, second;
    if (apint_is_greater(a, b)) {
        // a > b so a-b
        first = a;
        second = b;
        x->sign = a->sign;
    }
    else {
        // b > a so -(b-a)
        first = b;
        second = a;
        x->sign = -b->sign;
    }

    for (apint_size_t i = 0; i < first->length; i++) {
        x->limbs[i] = first->limbs[i] - second->limbs[i] - borrow;
        if (second->limbs[i] == UINT64_MAX && borrow) borrow = 1;
        else borrow = first->limbs[i] < second->limbs[i] + borrow;
    }

    return borrow;
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
/* Optimizations:
 * Changed to mid length
 * Unrolled by a factor of 4
 * Replaced the is greater comparison function call and unrolled it
 */
unsigned char apint_minus_optim1(apint_ptr x, apint_srcptr a, apint_srcptr b)
{
    assert(x->limbs && a->limbs && b->limbs);
    assert(a->length == b->length); // only handle same lengths for now
    assert(a->length <= x->length);
    unsigned char borrow = 0;
    int midlength = (b->length/2)+1;
    int is_greater1,is_greater2,is_greater3,is_greater4 ;
    int is_greater=0;
    unsigned char borrow1 = 0;
    unsigned char borrow2 = 0;
    unsigned char borrow3 = 0;
    unsigned char borrow4 = 0;
    for (int i = midlength; i >= 0; i-=4)
    {
        is_greater1=(a->limbs[i] > b->limbs[i]);
        is_greater2=(a->limbs[i-1] > b->limbs[i-1]);
        is_greater3=(a->limbs[i-2] > b->limbs[i-2]);
        is_greater4=(a->limbs[i-3] > b->limbs[i-3]);
        is_greater = is_greater | is_greater1|is_greater2|is_greater3|is_greater4;
    }
    if (is_greater) // a > b so a-b
    {
        x->sign = a->sign;
        for (apint_size_t i = 0; i < midlength; i+=4)
        {
            borrow1 = _subborrow_u64(borrow4, a->limbs[i], b->limbs[i], &x->limbs[i]);
            borrow2 = _subborrow_u64(borrow1, a->limbs[i+1], b->limbs[i+1], &x->limbs[i+1]);
            borrow3 = _subborrow_u64(borrow2, a->limbs[i+2], b->limbs[i+2], &x->limbs[i+2]);
            borrow4 = _subborrow_u64(borrow3, a->limbs[i+3], b->limbs[i+3], &x->limbs[i+3]);
        }
    }
    else // b > a so -(b-a)
    {
        x->sign = -b->sign;
        for (apint_size_t i = 0; i < midlength; i+=4)
        {
            borrow1 = _subborrow_u64(borrow4, b->limbs[i], a->limbs[i], &x->limbs[i]);
            borrow2 = _subborrow_u64(borrow1, b->limbs[i+1], a->limbs[i+1], &x->limbs[i+1]);
            borrow3 = _subborrow_u64(borrow2, b->limbs[i+2], a->limbs[i+2], &x->limbs[i+2]);
            borrow4 = _subborrow_u64(borrow3, b->limbs[i+3], a->limbs[i+3], &x->limbs[i+3]);
        }
    }
    return borrow4;
}

int apint_is_greater(apint_srcptr a, apint_srcptr b)
{
    //Works only for same length a, b
    for (int i = (a->length - 1); i >= 0; i--)
    {
        if (apint_getlimb(a, i) > apint_getlimb(b, i)) return 1;
    }
    return 0;
}

int apint_mul(apint_ptr x, apint_srcptr a, apint_srcptr b)
{
    assert(x->limbs && a->limbs && b->limbs);
    assert(a->length == b->length); // only handle same lengths for now
    assert(a->length == x->length);

    unsigned long long overflow;
    apint_limb_t b_limb;
    unsigned char carry;
    if (a->sign == b->sign) x->sign = 1;
    else x->sign = -1;

    for (apint_size_t i = 0; i < b->length; i++) {
        overflow = 0;
        carry = 0;
        b_limb = b->limbs[i];
        for (apint_size_t j = 0; j < a->length && i + j < x->length; j++) {
            carry = _addcarryx_u64(carry, x->limbs[i + j], overflow, &x->limbs[i + j]);
            carry += _addcarryx_u64(0, x->limbs[i + j], _mulx_u64(a->limbs[j], b_limb, &overflow), &x->limbs[i + j]);
        }
    }
    return (int) overflow;
}

int apint_mul_unroll(apint_ptr x, apint_srcptr a, apint_srcptr b)
{
    assert(x->limbs && a->limbs && b->limbs);
    assert(a->length == b->length); // only handle same lengths
    assert(a->length == x->length);

    if (a->sign == b->sign) x->sign = 1;
    else x->sign = -1;

    unsigned long long overflow;
    unsigned long long overflow1;
    unsigned long long overflow2;
    unsigned long long overflow3;
    unsigned long long overflow4;
    unsigned long long overflow5;
    unsigned long long overflow6;
    unsigned long long overflow7;
    unsigned long long temp;
    unsigned long long temp1;
    unsigned long long temp2;
    unsigned long long temp3;
    unsigned long long temp4;
    unsigned long long temp5;
    unsigned long long temp6;
    unsigned long long temp7;
    apint_limb_t b_limb;
    unsigned char carry;
    unsigned char carry1;
    unsigned char carry2;
    unsigned char carry3;
    unsigned char carry4;
    unsigned char carry5;
    unsigned char carry6;
    unsigned char carry7;
    if (a->length < 4) {
        // a and b should be same length
        return apint_mul(x, a, b);
    }
    else {
        // Loop unrolling if size is greater than 4
        for (apint_size_t i = 0; i < b->length; i += 1) {
            // doing 1 for now
            overflow = 0;
            carry = 0;
            b_limb = b->limbs[i];
            apint_size_t j = 0;
            for (; j < a->length && i + j + 7 < x->length; j += 8) {
                carry1 = _addcarryx_u64(carry, x->limbs[i + j], overflow, &x->limbs[i + j]); // needs to be done first because dependent on previous overflow
                temp = _mulx_u64(a->limbs[j], b_limb, &overflow1);
                temp1 = _mulx_u64(a->limbs[j + 1], b_limb, &overflow2);
                temp2 = _mulx_u64(a->limbs[j + 2], b_limb, &overflow3);
                temp3 = _mulx_u64(a->limbs[j + 3], b_limb, &overflow4);
                temp4 = _mulx_u64(a->limbs[j + 4], b_limb, &overflow5);
                temp5 = _mulx_u64(a->limbs[j + 5], b_limb, &overflow6);
                temp6 = _mulx_u64(a->limbs[j + 6], b_limb, &overflow7);
                temp7 = _mulx_u64(a->limbs[j + 7], b_limb, &overflow);

                carry1 += _addcarryx_u64(0, x->limbs[i + j], temp, &x->limbs[i + j]);

                carry2 = _addcarryx_u64(carry1, x->limbs[i + j + 1], overflow1, &x->limbs[i + j + 1]);
                carry2 += _addcarryx_u64(0, x->limbs[i + j + 1], temp1, &x->limbs[i + j + 1]);

                carry3 = _addcarryx_u64(carry2, x->limbs[i + j + 2], overflow2, &x->limbs[i + j + 2]);
                carry3 += _addcarryx_u64(0, x->limbs[i + j + 2], temp2, &x->limbs[i + j + 2]);

                carry4 = _addcarryx_u64(carry3, x->limbs[i + j + 3], overflow3, &x->limbs[i + j + 3]);
                carry4 += _addcarryx_u64(0, x->limbs[i + j + 3], temp3, &x->limbs[i + j + 3]);

                carry5 = _addcarryx_u64(carry4, x->limbs[i + j + 4], overflow4, &x->limbs[i + j + 4]);
                carry5 += _addcarryx_u64(0, x->limbs[i + j + 4], temp4, &x->limbs[i + j + 4]);

                carry6 = _addcarryx_u64(carry5, x->limbs[i + j + 5], overflow5, &x->limbs[i + j + 5]);
                carry6 += _addcarryx_u64(0, x->limbs[i + j + 5], temp5, &x->limbs[i + j + 5]);

                carry7 = _addcarryx_u64(carry6, x->limbs[i + j + 6], overflow6, &x->limbs[i + j + 6]);
                carry7 += _addcarryx_u64(0, x->limbs[i + j + 6], temp6, &x->limbs[i + j + 6]);

                carry = _addcarryx_u64(carry7, x->limbs[i + j + 7], overflow7, &x->limbs[i + j + 7]);
                carry += _addcarryx_u64(0, x->limbs[i + j + 7], temp7, &x->limbs[i + j + 7]);
            }
            for (; j < a->length && i + j < x->length; j++) {
                carry = _addcarryx_u64(carry, x->limbs[i + j], overflow, &x->limbs[i + j]);
                carry += _addcarryx_u64(0, x->limbs[i + j], _mulx_u64(a->limbs[j], b_limb, &overflow), &x->limbs[i + j]);
            }
        }
    }

    return (int) overflow;
}

int apint_mul_OPT1(apint_ptr x, apint_srcptr a, apint_srcptr b)
{
    //TODO: check if these checks are needed anywhere else in the code.
    assert(x->limbs && a->limbs && b->limbs);
    assert(a->length == b->length); // only handle same lengths
    assert(a->length == x->length);

    unsigned long long overflow;
    unsigned long long overflow1;
    unsigned long long overflow2;
    unsigned long long overflow3;
    unsigned long long temp;
    unsigned long long temp1;
    unsigned long long temp2;
    unsigned long long temp3;
    unsigned char carry;
    unsigned char carry1;
    unsigned char carry2;
    unsigned char carry3;
    if (a->sign == b->sign)
        x->sign = 1;
    else
        x->sign = -1;

    if (a->length <= 4 || b->length <= 4) // a and b should be same length
    {
        return apint_mul(x, a, b);
    }
    else // Loop unrolling if size is greater than 4
    {
        for (apint_size_t i = 0; i < b->length; i += 1) // doing 1 for now
        {
            overflow = 0;
            carry = 0;
            for (apint_size_t j = 0; j < a->length; j += 4)
            {
                // make sure we don't try to set something in x that is outside of its precision
                if ((i + j + 3) < x->length)
                {
                    carry1 = _addcarryx_u64(carry, x->limbs[i + j], overflow, &x->limbs[i + j]); // needs to be done first because dependent on previous overflow
                    temp = _mulx_u64(a->limbs[j], b->limbs[i], &overflow1);
                    temp1 = _mulx_u64(a->limbs[j + 1], b->limbs[i], &overflow2);
                    temp2 = _mulx_u64(a->limbs[j + 2], b->limbs[i], &overflow3);
                    temp3 = _mulx_u64(a->limbs[j + 3], b->limbs[i], &overflow);

                    carry1 += _addcarryx_u64(0, x->limbs[i + j], temp, &x->limbs[i + j]);

                    carry2 = _addcarryx_u64(carry1, x->limbs[i + j + 1], overflow1, &x->limbs[i + j + 1]);
                    carry2 += _addcarryx_u64(0, x->limbs[i + j + 1], temp1, &x->limbs[i + j + 1]);

                    carry3 = _addcarryx_u64(carry2, x->limbs[i + j + 2], overflow2, &x->limbs[i + j + 2]);
                    carry3 += _addcarryx_u64(0, x->limbs[i + j + 2], temp2, &x->limbs[i + j + 2]);

                    carry = _addcarryx_u64(carry3, x->limbs[i + j + 3], overflow3, &x->limbs[i + j + 3]);
                    carry += _addcarryx_u64(0, x->limbs[i + j + 3], temp3, &x->limbs[i + j + 3]);
                }
            }
        }
    }

    return (int)overflow;
}

void apint_mul_portable(apint_ptr x, apint_srcptr a, apint_srcptr b)
{
    assert(x->limbs && a->limbs && b->limbs);
    assert(a->length == b->length); // only handle same lengths for now
    assert(a->length == x->length);

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
        overflow = 0;
        for (size_t a_i = 0; a_i < a->length * 2; a_i++)
        {
            if ((a_i + b_i) < x->length * 2) {
                x_vals[a_i + b_i] += overflow;
                uint64_t res = (uint64_t) a_vals[a_i] * (uint64_t) b_vals[b_i];
                overflow = res >> 32;
                x_vals[a_i + b_i] += res;
            }
        }
    }
}

void apint_div(apint_ptr x, apint_srcptr a, apint_srcptr b)
{
    // To-do: Implement division.

    // keep trying greatest powers of 2 and if it doesnt work shift, otherwise multiply (well its by 1 anyways), but just subtract
}

/*
Was supposed to be used for karatsuba adding when two lengths are different, but apint_add seems to work?
*/
char apint_add_karatsuba(apint_ptr x, apint_srcptr a, apint_srcptr b)
{
    assert(x->limbs && a->limbs && b->limbs);
    assert((max(a->length, b->length)) == x->length);
    /*
    a has to be the bigger one because when we pass in a_high,
    it will always be greater or equal because of how we calculate "d"
    */
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

uint64_t apint_mul_karatsuba(apint_ptr x, apint_srcptr a, apint_srcptr b)
{
    assert(x->limbs && a->limbs && b->limbs);
    assert(a->length == b->length); // only handle same lengths of input
    assert(a->length == x->length); // assuming that output has same precision as both inputs

    // Make sure to keep track of the sign, but pass everything in as positive
    x->sign = a->sign * b->sign;

    // if lengths small enough, return a*b
    if (a->length <= 1 || b->length <= 1) // they have to be the same length anyways
        return apint_mul_karatsuba_base_case(x, a, b);

    uint64_t overflow = apint_mul_karatsuba_recurse(x, a, b); // Although I don't think there will be overflow here
    return overflow;                                          // this returns a unit64_t
}

/*
This is a recursive method
*/
uint64_t apint_mul_karatsuba_recurse(apint_ptr x, apint_srcptr a, apint_srcptr b)
{
    // if lengths small enough, return a*b
    // karatsuba_base_case handles different precision input and output because it is needed
    if (a->length == 1 || b->length == 1)
        return apint_mul_karatsuba_base_case(x, a, b);

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
    z0->sign = 1;
    z1->sign = 1;
    z2->sign = 1;
    apint_init(z0, x->length * 64); // Padding it to oblivion, trade off between performance and precision
    apint_init(z1, x->length * 64);
    apint_init(z2, x->length * 64);

    apint_t a_add, b_add;
    a_add->sign = 1;
    b_add->sign = 1;
    char a_add_overflow, b_add_overflow;
    apint_init(a_add, (max(a_high->length, a_low->length)) * 64);
    apint_init(b_add, (max(b_high->length, b_low->length)) * 64);

    a_add_overflow = apint_add(a_add, a_high, a_low); // a_high and a_low have to be the same length for now ASSUMPTION
    b_add_overflow = apint_add(b_add, b_high, b_low); // a_high and a_low have to be the same length for now

    apint_mul_karatsuba_recurse(z0, a_low, b_low);
    apint_mul_karatsuba_recurse(z1, a_add, b_add);   // THE LENGTH NEVER DECREASES, ok now it decreases, so its fine
    apint_mul_karatsuba_recurse(z2, a_high, b_high); // There should be an overflow but I don't think I need to do anything with it

    // FREE THINGS
    apint_free(a_high);
    apint_free(a_low);
    apint_free(b_high);
    apint_free(b_low);
    apint_free(a_add);
    apint_free(b_add);

    apint_t first_operand; // z2 + z0
    first_operand->sign = 1;
    apint_init(first_operand, (z2->length) * 64);
    apint_add(first_operand, z2, z0);

    apint_t second_operand; // z1 - (z2 + z0)
    second_operand->sign = 1;
    apint_init(second_operand, (z1->length) * 64);
    apint_sub(second_operand, z1, first_operand);

    // Shift results appropriately, should be stored in z2 and second_operand
    d = d * 64; // in the beginning we split by d, but d is a limb, which is 64 bits

    // INLINE THE SHIFTS
    apint_shiftl(z2, (2 * d));       // multiply by 2 because of Karatsuba algorithm
    apint_shiftl(second_operand, d); // TODO: INLINE

    // x = z2 * 2 ^ (2 * d) + (z1 - z2 - z0) * 2 ^ (d) + z0;
    apint_t temp_x;
    apint_init(temp_x, (x->length) * 64);
    apint_add(temp_x, z2, second_operand);
    apint_add(x, temp_x, z0);

    // FREE EVERYTHING ELSE
    apint_free(z0);
    apint_free(z1);
    apint_free(z2);
    apint_free(first_operand);
    apint_free(second_operand);
    apint_free(temp_x);

    uint64_t result; // not used
    return result;
}

uint64_t apint_mul_karatsuba_base_case(apint_ptr x, apint_srcptr a, apint_srcptr b)
{
    assert(x->limbs && a->limbs && b->limbs);
    assert(a->length == b->length); // only handle same lengths for now
    // assert((a->length + b->length) <= x->length); // output can be different precision than input due to recursion and not wanting to lose precision during recursion

    unsigned long long overflow;
    unsigned char carry;
    if (a->sign == b->sign)
        x->sign = 1;
    else
        x->sign = -1;
    for (apint_size_t i = 0; i < b->length; i++)
    {
        overflow = 0;
        carry = 0;
        for (apint_size_t j = 0; j < a->length; j++)
        {
            // make sure we don't try to set something in x that is outside of its precision
            if ((i + j) < x->length)
            {
                carry = _addcarryx_u64(carry, x->limbs[i + j], overflow, &x->limbs[i + j]);
                x->limbs[i + j] += carry;
                x->limbs[i + j] += _mulx_u64(a->limbs[j], b->limbs[i], &overflow);
            }
        }
    }
    return (int)overflow;
}

/* -------------------------------------- OPTIMIZATIONS BELOW (Extend Recursive Basecase) --------------------------------------  */
uint64_t apint_mul_karatsuba_extend_basecase(apint_ptr x, apint_srcptr a, apint_srcptr b)
{
    assert(x->limbs && a->limbs && b->limbs);
    assert(a->length == b->length); // only handle same lengths of input
    assert(a->length == x->length); // assuming that output has same precision as both inputs

    // Make sure to keep track of the sign, but pass everything in as positive
    x->sign = a->sign * b->sign;

    // if lengths small enough, return a*b
    if (a->length <= 1 || b->length <= 1) // they have to be the same length anyways
        return apint_mul_karatsuba_base_case(x, a, b);

    uint64_t overflow = apint_mul_karatsuba_recurse_extend_basecase(x, a, b); // Although I don't think there will be overflow here
    return overflow;                                                          // this returns a unit64_t
}

/*
Trying to "extend the base case" to see if that adds any improvement
Also used to PROFILE optimizations in:
apint_add
apint_copyover
apint_sub
*/
uint64_t apint_mul_karatsuba_recurse_extend_basecase(apint_ptr x, apint_srcptr a, apint_srcptr b)
{
    // if lengths small enough, return a*b
    // karatsuba_base_case handles different precision input and output because it is needed
    // 10 saw improvements
    if (a->length <= 8 || b->length <= 8)
        return apint_mul_karatsuba_base_case(x, a, b);

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
    z0->sign = 1;
    z1->sign = 1;
    z2->sign = 1;
    apint_init(z0, x->length * 64); // Padding it to oblivion, trade off between performance and precision
    apint_init(z1, x->length * 64);
    apint_init(z2, x->length * 64);

    apint_t a_add, b_add;
    a_add->sign = 1;
    b_add->sign = 1;
    char a_add_overflow, b_add_overflow;
    apint_init(a_add, (max(a_high->length, a_low->length)) * 64);
    apint_init(b_add, (max(b_high->length, b_low->length)) * 64);

    a_add_overflow = apint_add(a_add, a_high, a_low); // a_high and a_low have to be the same length for now ASSUMPTION
    b_add_overflow = apint_add(b_add, b_high, b_low); // a_high and a_low have to be the same length for now

    apint_mul_karatsuba_recurse_extend_basecase(z0, a_low, b_low);
    apint_mul_karatsuba_recurse_extend_basecase(z1, a_add, b_add);   // THE LENGTH NEVER DECREASES, ok now it decreases, so its fine
    apint_mul_karatsuba_recurse_extend_basecase(z2, a_high, b_high); // There should be an overflow but I don't think I need to do anything with it

    // FREE THINGS
    apint_free(a_high);
    apint_free(a_low);
    apint_free(b_high);
    apint_free(b_low);
    apint_free(a_add);
    apint_free(b_add);

    apint_t first_operand; // z2 + z0
    first_operand->sign = 1;
    apint_init(first_operand, (z2->length) * 64);
    apint_add(first_operand, z2, z0);

    apint_t second_operand; // z1 - (z2 + z0)
    second_operand->sign = 1;
    apint_init(second_operand, (z1->length) * 64);
    apint_sub(second_operand, z1, first_operand);

    // Shift results appropriately, should be stored in z2 and second_operand
    d = d * 64;                // in the beginning we split by d, but d is a limb, which is 64 bits
    apint_shiftl(z2, (2 * d)); // multiply by 2 because of Karatsuba algorithm
    apint_shiftl(second_operand, d);

    // x = z2 * 2 ^ (2 * d) + (z1 - z2 - z0) * 2 ^ (d) + z0;
    apint_t temp_x;
    apint_init(temp_x, (x->length) * 64);
    apint_add(temp_x, z2, second_operand);
    apint_add(x, temp_x, z0);

    // FREE EVERYTHING ELSE
    apint_free(z0);
    apint_free(z1);
    apint_free(z2);
    apint_free(first_operand);
    apint_free(second_operand);
    apint_free(temp_x);

    uint64_t result; // not used yet
    return result;
}

/* -------------------------------------- OPTIMIZATIONS BELOW (Karatsuba ILP) --------------------------------------  */
uint64_t apint_mul_karatsuba_OPT1(apint_ptr x, apint_srcptr a, apint_srcptr b)
{
    assert(x->limbs && a->limbs && b->limbs);
    assert(a->length == b->length); // only handle same lengths of input
    assert(a->length == x->length); // assuming that output has same precision as both inputs

    // Make sure to keep track of the sign, but pass everything in as positive
    x->sign = a->sign * b->sign;

    // if lengths small enough, return a*b
    if (a->length <= 1 || b->length <= 1) // they have to be the same length anyways
        return apint_mul_karatsuba_base_case(x, a, b);

    uint64_t overflow = apint_mul_karatsuba_recurse_OPT1(x, a, b); // Although I don't think there will be overflow here
    return overflow;                                               // this returns a unit64_t
}

/*
This is a recursive method, it seems like apint_add is still the bottleneck
- benefits are from loops for inlining, method calls in loops
*/
uint64_t apint_mul_karatsuba_recurse_OPT1(apint_ptr x, apint_srcptr a, apint_srcptr b)
{
    // if lengths small enough, return a*b
    // karatsuba_base_case handles different precision input and output because it is needed
    if (a->length <= 8 || b->length <= 8)
        return apint_mul_karatsuba_base_case(x, a, b);

    // d = floor(max(length(a), length(b)) / 2)
    apint_size_t d = floor(max(a->length, b->length) / 2); // They're the same length anyways

    // x_high, x_low = split x at d, or right shift by d
    apint_t a_high, a_low;
    a_high->sign = 1;
    a_low->sign = 1;
    apint_init(a_high, (a->length - d) * 64); // The 64 bits here is under the assumption that we use ints to represent everything
    apint_init(a_low, d * 64);                // a_low length will always be smaller or equal to a_high length bc of d

    // apint_copyover(a_low, a, 0); // INLINE BELOW
    // apint_copyover(a_high, a, d);

    // y_high, y_low = split y at d, or right shift by d
    apint_t b_high, b_low;
    b_high->sign = 1;
    b_low->sign = 1;
    apint_init(b_high, (b->length - d) * 64);
    apint_init(b_low, d * 64);

    // apint_copyover(b_low, b, 0); // INLINE BELOW
    // apint_copyover(b_high, b, d);
    int i;
    for (i = 0; i < a_low->length; i++) // INLINE for apint_copyover
    {
        a_low->limbs[i] = a->limbs[i];
        a_high->limbs[i] = a->limbs[i + d];
        b_low->limbs[i] = b->limbs[i];
        b_high->limbs[i] = b->limbs[i + d];
    }

    for (; i < a_high->length; i++)
    {
        a_high->limbs[i] = a->limbs[i + d];
        b_high->limbs[i] = b->limbs[i + d];
    }

    apint_t z0, z1, z2;
    z0->sign = 1;
    z1->sign = 1;
    z2->sign = 1;
    apint_init(z0, x->length * 64); // Padding it to oblivion, trade off between performance and precision
    apint_init(z1, x->length * 64);
    apint_init(z2, x->length * 64);

    apint_t a_add, b_add;
    a_add->sign = 1;
    b_add->sign = 1;
    char a_add_overflow, b_add_overflow;
    apint_init(a_add, (max(a_high->length, a_low->length)) * 64);
    apint_init(b_add, (max(b_high->length, b_low->length)) * 64);

    a_add_overflow = apint_add(a_add, a_high, a_low); // a_high and a_low have to be the same length for now ASSUMPTION
    b_add_overflow = apint_add(b_add, b_high, b_low); // a_high and a_low have to be the same length for now

    apint_mul_karatsuba_recurse_OPT1(z0, a_low, b_low);
    apint_mul_karatsuba_recurse_OPT1(z1, a_add, b_add);   // THE LENGTH NEVER DECREASES, ok now it decreases, so its fine
    apint_mul_karatsuba_recurse_OPT1(z2, a_high, b_high); // There should be an overflow but I don't think I need to do anything with it

    // FREE THINGS
    apint_free(a_high);
    apint_free(a_low);
    apint_free(b_high);
    apint_free(b_low);
    apint_free(a_add);
    apint_free(b_add);

    apint_t first_operand; // z2 + z0
    first_operand->sign = 1;
    apint_init(first_operand, (z2->length) * 64);
    apint_add(first_operand, z2, z0);

    apint_t second_operand; // z1 - (z2 + z0)
    second_operand->sign = 1;
    apint_init(second_operand, (z1->length) * 64);
    apint_sub(second_operand, z1, first_operand);

    // Shift results appropriately, should be stored in z2 and second_operand
    d = d * 64; // in the beginning we split by d, but d is a limb, which is 64 bits
    // INLINE THE SHIFTS
    // apint_shiftl(z2, (2 * d));       // multiply by 2 because of Karatsuba algorithm
    uint full_limbs_shifted = d / APINT_LIMB_BITS;
    d -= full_limbs_shifted * APINT_LIMB_BITS;
    for (int i = z2->length - 1; i >= 0; i--)
    {
        if (i - (int)full_limbs_shifted >= 0)
        {
            z2->limbs[i] = z2->limbs[i - full_limbs_shifted];
        }

        else
        {
            z2->limbs[i] = 0;
        }
    }
    for (int i = z2->length - 1; i > 0; i--)
    {
        z2->limbs[i] = (z2->limbs[i] << d) + (z2->limbs[i - 1] >> (APINT_LIMB_BITS - d));
    }
    z2->limbs[0] <<= d;

    // apint_shiftl(second_operand, d); // TODO: INLINE
    // uint full_limbs_shifted = d / APINT_LIMB_BITS;
    // d -= full_limbs_shifted * APINT_LIMB_BITS;
    for (int i = second_operand->length - 1; i >= 0; i--)
    {
        if (i - (int)full_limbs_shifted >= 0)
        {
            second_operand->limbs[i] = second_operand->limbs[i - full_limbs_shifted];
        }

        else
        {
            second_operand->limbs[i] = 0;
        }
    }
    for (int i = second_operand->length - 1; i > 0; i--)
    {
        second_operand->limbs[i] = (second_operand->limbs[i] << d) + (second_operand->limbs[i - 1] >> (APINT_LIMB_BITS - d));
    }
    second_operand->limbs[0] <<= d;

    // x = z2 * 2 ^ (2 * d) + (z1 - z2 - z0) * 2 ^ (d) + z0;
    apint_t temp_x;
    apint_init(temp_x, (x->length) * 64);
    apint_add(temp_x, z2, second_operand);
    apint_add(x, temp_x, z0);

    // FREE EVERYTHING ELSE
    apint_free(z0);
    apint_free(z1);
    apint_free(z2);
    apint_free(first_operand);
    apint_free(second_operand);
    apint_free(temp_x);

    uint64_t result; // not used yet
    return result;
}