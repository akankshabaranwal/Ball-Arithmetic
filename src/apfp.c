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

static inline bool adjust_alignment_base(apfp_ptr x)
{
    bool is_exact = true;
    size_t overflow = apint_detectfirst1(x->mant);

    if (overflow > MID_POS_BITWISE(x))
    {
        overflow -= MID_POS_BITWISE(x);
        is_exact = !apint_shiftr(x->mant, overflow);
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

//first optimization
static inline bool adjust_alignment(apfp_ptr x)
{
    bool is_exact = true;
    size_t overflow = apint_detectfirst1(x->mant);

    int mid_pos_bitwise_val = MID_POS_BITWISE(x);
    if (overflow > mid_pos_bitwise_val)
    {
        overflow -= mid_pos_bitwise_val;
        is_exact = apint_shiftr(x->mant, overflow);
        x->exp += (apfp_exp_t) overflow;
    }
    else if (overflow < mid_pos_bitwise_val)
    {
        // Can't shift off bits here
        overflow = mid_pos_bitwise_val - overflow;
        apint_shiftl(x->mant, overflow);
        x->exp -= (apfp_exp_t) overflow;
    }
    return is_exact;
}

bool apfp_add_base(apfp_ptr x, apfp_srcptr a, apfp_srcptr b)
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

    // Add mantissa, shift by carry and update exponent
    apint_add(x->mant, x->mant, a->mant);
    x->exp = a->exp;
    if(MIDDLE_LEFT(x) != 0 && (apint_getlimb(x->mant, 0) & 0x1ull) != 0)
        is_exact = false;

    adjust_alignment(x);

    return is_exact;
}

// Collapsed all functions to 1.
bool apfp_add_optim1(apfp_ptr x, apfp_srcptr a, apfp_srcptr b)
{
    assert(x->mant->length == a->mant->length);
    assert(x->mant->length == b->mant->length);

    bool is_exact = true;
    // After swap, `a` is guaranteed to have largest exponent
    if (b->exp > a->exp)
    {
        apfp_srcptr t = a; a = b; b = t;
    }

    // Align `b` mantissa to `a` given exponent difference
    apfp_exp_t factor = a->exp - b->exp;

    // We could easily combine shift and copy here

    for (int i = 0; i < b->mant->length; i+=4)
    {
        x->mant->limbs[i] = b->mant->limbs[i];
        x->mant->limbs[i+1] = b->mant->limbs[i+1];
        x->mant->limbs[i+2] = b->mant->limbs[i+2];
        x->mant->limbs[i+3] = b->mant->limbs[i+3];
    }
    x->mant->sign = b->mant->sign;

    if(factor)
    {
    int full_limbs_shifted = factor / APINT_LIMB_BITS;
    factor -= full_limbs_shifted * APINT_LIMB_BITS;

    int full_limbs_shifted_1 = full_limbs_shifted-1;
    for (int i = full_limbs_shifted; i  < x->mant->length; i+=2)
    {
        x->mant->limbs[i-full_limbs_shifted] = x->mant->limbs[i];
        x->mant->limbs[i-full_limbs_shifted_1] = x->mant->limbs[i+1];
    }

    if (factor)
        {
        int leftshiftamt = (APINT_LIMB_BITS - factor);
        for (int i = 0; i < x->mant->length - 1; ++i)
        {
            x->mant->limbs[i] = (x->mant->limbs[i] >> factor) + (x->mant->limbs[i + 1] << leftshiftamt);
        }
        x->mant->limbs[x->mant->length - 1] >>= factor;
        }
    }

    // Add mantissa, shift by carry and update exponent

    if (x->mant->sign == a->mant->sign)
    {
        apint_plus(x->mant, x->mant, a->mant);
        x->mant->sign = a->mant->sign;
    }
    else
    {
        if (x->mant->sign == 1) //only a is negative. so equivalent to b-a.
        {
            apint_minus(x->mant, x->mant, a->mant);
        }
        else
        {
            apint_minus(x->mant, a->mant, x->mant);
        }
    }

    x->exp = a->exp;
    if(MIDDLE_LEFT(x) != 0 && (x->mant->limbs[0]& 0x1ull) != 0)
        is_exact = false;

    //is_exact = adjust_alignment(x);
    size_t overflow = apint_detectfirst1(x->mant);
    int mid_pos_bitwise_val = MID_POS_BITWISE(x);
    if (overflow > mid_pos_bitwise_val)
    {
        overflow -= mid_pos_bitwise_val;
        //is_exact = apint_shiftr(x->mant, overflow);
        if (overflow)
        {
            int full_limbs_shifted = overflow / APINT_LIMB_BITS;
            overflow -= full_limbs_shifted * APINT_LIMB_BITS;
            int full_limbs_shifted_1 = full_limbs_shifted - 1;
            for (int i = full_limbs_shifted; i < x->mant->length; i += 2) {
                x->mant->limbs[i - full_limbs_shifted] = x->mant->limbs[i];
                x->mant->limbs[i - full_limbs_shifted_1] = x->mant->limbs[i + 1];
            }
            if (overflow)
            {
                int leftshiftamt = (APINT_LIMB_BITS - overflow);
                for (int i = 0; i < x->mant->length - 1; ++i)
                {
                    x->mant->limbs[i] = (x->mant->limbs[i] >> overflow) + (x->mant->limbs[i + 1] << leftshiftamt);
                }
                    x->mant->limbs[x->mant->length - 1] >>= overflow;
            }
        }
        x->exp += (apfp_exp_t) overflow;
    }
    else if (overflow < mid_pos_bitwise_val)
    {
        // Can't shift off bits here
        overflow = mid_pos_bitwise_val - overflow;
        if (overflow) {
            int full_limbs_shifted = overflow / APINT_LIMB_BITS;
            overflow -= full_limbs_shifted * APINT_LIMB_BITS;
            int full_limbs_shifted_1 = full_limbs_shifted + 1;
            for (int i = x->mant->length - 1; i >= full_limbs_shifted - 1; i -= 2) {
                x->mant->limbs[i] = x->mant->limbs[i - full_limbs_shifted];
                //x->limbs[i-1] = x->limbs[i-full_limbs_shifted-1];
                x->mant->limbs[i - 1] = x->mant->limbs[i - full_limbs_shifted_1];
            }

            if (overflow) {
                int rightshiftamt = (APINT_LIMB_BITS - overflow);
                for (int i = x->mant->length - 1; i > 0; i--) {
                    x->mant->limbs[i] = (x->mant->limbs[i] << overflow) + (x->mant->limbs[i - 1] >> rightshiftamt);
                }
                x->mant->limbs[0] <<= overflow;
            }
        }
        x->exp -= (apfp_exp_t) overflow;
    }
    return is_exact;
}

// Collapsed all functions to 1.
bool apfp_add_optim2(apfp_ptr x, apfp_srcptr a, apfp_srcptr b)
{
    assert(x->mant->length == a->mant->length);
    assert(x->mant->length == b->mant->length);

    bool is_exact = true;
    // After swap, `a` is guaranteed to have largest exponent
    if (b->exp > a->exp){
        apfp_srcptr t = a; a = b; b = t;
    }

    // Align `b` mantissa to `a` given exponent difference
    apfp_exp_t factor = a->exp - b->exp;
    int maxlength = b->mant->length;
    int midlength = (b->mant->length/2)+1;
    for (int i = 0; i < midlength; i+=4){
        x->mant->limbs[i] = b->mant->limbs[i];
        x->mant->limbs[i+1] = b->mant->limbs[i+1];
        x->mant->limbs[i+2] = b->mant->limbs[i+2];
        x->mant->limbs[i+3] = b->mant->limbs[i+3];
    }
    x->mant->sign = b->mant->sign;

    int full_limbs_shifted = factor / APINT_LIMB_BITS;
    factor -= full_limbs_shifted * APINT_LIMB_BITS;

    int full_limbs_shifted_1 = full_limbs_shifted-1;
    // right shift
    for (int i = full_limbs_shifted; i  < maxlength; i+=2){
        x->mant->limbs[i-full_limbs_shifted] = x->mant->limbs[i];
        x->mant->limbs[i-full_limbs_shifted_1] = x->mant->limbs[i+1];
    }
    if (factor){
        int leftshiftamt = (APINT_LIMB_BITS - factor);
        for (int i = 0; i < maxlength - 1; ++i){
            x->mant->limbs[i] = (x->mant->limbs[i] >> factor) + (x->mant->limbs[i + 1] << leftshiftamt);
        }
        x->mant->limbs[maxlength - 1] >>= factor;
    }

    // Add mantissa, shift by carry and update exponent
    if (x->mant->sign == a->mant->sign){
        apint_plus(x->mant, x->mant, a->mant);
        x->mant->sign = a->mant->sign;
    }
    else{
        if (x->mant->sign == 1){
            apint_minus(x->mant, x->mant, a->mant);
        }
        else{
            apint_minus(x->mant, a->mant, x->mant);
        }
    }

    x->exp = a->exp;
    if(MIDDLE_LEFT(x) != 0)
        is_exact = false;

    size_t pos;
    size_t i;
    for(i = midlength; i >= 0; i--)
    {
        if(x->mant->limbs[i] > 0)
        {
            break;
        }
    }
    pos = APINT_LIMB_BITS * (maxlength-i-1);
    int xsize = maxlength * APINT_LIMB_BITS;
    int bitpos = __builtin_clzll(x->mant->limbs[i]);
    pos = pos + bitpos;
    size_t overflow = (xsize-pos);

    int mid_pos_bitwise_val = MID_POS_BITWISE(x);
    if (overflow > mid_pos_bitwise_val){
        overflow -= mid_pos_bitwise_val;
            int full_limbs_shifted = overflow / APINT_LIMB_BITS;
            overflow -= full_limbs_shifted * APINT_LIMB_BITS;
            int full_limbs_shifted_1 = full_limbs_shifted - 1;
            for (int i = full_limbs_shifted; i < maxlength; i += 2) {
                x->mant->limbs[i - full_limbs_shifted] = x->mant->limbs[i];
                x->mant->limbs[i - full_limbs_shifted_1] = x->mant->limbs[i + 1];
            }
            if (overflow){
                int leftshiftamt = (APINT_LIMB_BITS - overflow);
                for (int i = 0; i < maxlength-1; ++i)
                {
                    x->mant->limbs[i] = (x->mant->limbs[i] >> overflow) + (x->mant->limbs[i + 1] << leftshiftamt);
                }
                x->mant->limbs[x->mant->length - 1] >>= overflow;
            }
        x->exp += (apfp_exp_t) overflow;
    }
    else if (overflow < mid_pos_bitwise_val){
        overflow = mid_pos_bitwise_val - overflow;
        if (overflow) {
            int full_limbs_shifted = overflow / APINT_LIMB_BITS;
            overflow -= full_limbs_shifted * APINT_LIMB_BITS;
            int full_limbs_shifted_1 = full_limbs_shifted + 1;
            for (int i = maxlength - 1; i >= full_limbs_shifted - 1; i -= 2) {
                x->mant->limbs[i] = x->mant->limbs[i - full_limbs_shifted];
                x->mant->limbs[i - 1] = x->mant->limbs[i - full_limbs_shifted_1];
            }

            if (overflow) {
                int rightshiftamt = (APINT_LIMB_BITS - overflow);
                for (int i = maxlength - 1; i > 0; i--) {
                    x->mant->limbs[i] = (x->mant->limbs[i] << overflow) + (x->mant->limbs[i - 1] >> rightshiftamt);
                }
                x->mant->limbs[0] <<= overflow;
            }
        }
        x->exp -= (apfp_exp_t) overflow;
    }
    return is_exact;
}


// shiftr is the bottleneck now, mid pt
bool apfp_add_optim3(apfp_ptr x, apfp_srcptr a, apfp_srcptr b)
{
    assert(x->mant->length == a->mant->length);
    assert(x->mant->length == b->mant->length);

    bool is_exact = true;
    // After swap, `a` is guaranteed to have largest exponent
    if (b->exp > a->exp){
        apfp_srcptr t = a; a = b; b = t;
    }

    // Align `b` mantissa to `a` given exponent difference
    apfp_exp_t factor = a->exp - b->exp;
    int maxlength = b->mant->length;
    int midlength = (b->mant->length/2)+1;
    for (int i = 0; i < midlength; i+=4){
        x->mant->limbs[i] = b->mant->limbs[i];
        x->mant->limbs[i+1] = b->mant->limbs[i+1];
        x->mant->limbs[i+2] = b->mant->limbs[i+2];
        x->mant->limbs[i+3] = b->mant->limbs[i+3];
    }
    x->mant->sign = b->mant->sign;

    int full_limbs_shifted = factor / APINT_LIMB_BITS;
    factor -= full_limbs_shifted * APINT_LIMB_BITS;

    int full_limbs_shifted_1 = full_limbs_shifted-1;
    // right shift
    for (int i = full_limbs_shifted; i  < maxlength; i+=2){
        x->mant->limbs[i-full_limbs_shifted] = x->mant->limbs[i];
        x->mant->limbs[i-full_limbs_shifted_1] = x->mant->limbs[i+1];
    }
    if (factor){
        int leftshiftamt = (APINT_LIMB_BITS - factor);
        for (int i = 0; i < midlength; ++i){
            x->mant->limbs[i] = (x->mant->limbs[i] >> factor) + (x->mant->limbs[i + 1] << leftshiftamt);
        }
    }
    size_t pos;
    size_t i;
    size_t overflow;

    int mid_pos_bitwise_val = MID_POS_BITWISE(x);
    // Add mantissa, shift by carry and update exponent
    x->exp = a->exp;
    unsigned char carry1 = 0;
    unsigned char carry2 = 0;
    unsigned char carry3 = 0;
    unsigned char carry4 = 0;

    if (x->mant->sign == a->mant->sign)
    {
        for (apint_size_t i = 0; i < midlength; i++)
        {
            carry1 = _addcarryx_u64(carry1, x->mant->limbs[i], a->mant->limbs[i], &x->mant->limbs[i]);
        }

        x->mant->sign = a->mant->sign;
        for(i = midlength; i >= 0; i--)
        {
            if(x->mant->limbs[i] > 0)
            {
                break;
            }
        }
        pos = APINT_LIMB_BITS * (maxlength-i-1);
        int xsize = maxlength * APINT_LIMB_BITS;
        int bitpos = __builtin_clzll(x->mant->limbs[i]);
        pos = pos + bitpos;
        overflow = (xsize-pos);
        if (overflow > mid_pos_bitwise_val)
        {
            overflow -= mid_pos_bitwise_val;
            int leftshiftamt = (APINT_LIMB_BITS - overflow);
            for (int i = 0; i < midlength; ++i)
            {
                x->mant->limbs[i] = (x->mant->limbs[i] >> overflow) + (x->mant->limbs[i + 1] << leftshiftamt);
            }
            x->mant->limbs[x->mant->length - 1] >>= overflow;
            x->exp += (apfp_exp_t) overflow;
        }
        else if (overflow < mid_pos_bitwise_val)
        {
            overflow = mid_pos_bitwise_val - overflow;
            int rightshiftamt = (APINT_LIMB_BITS - overflow);
            for (i = midlength ; i > 0; i--) {
                x->mant->limbs[i] = (x->mant->limbs[i] << overflow) + (x->mant->limbs[i - 1] >> rightshiftamt);
            }
            x->mant->limbs[0] <<= overflow;
            x->exp -= (apfp_exp_t) overflow;
        }
    }
    else{
        if (x->mant->sign == 1)
        {
            apint_minus(x->mant, x->mant, a->mant);
        }
        else{
            apint_minus(x->mant, a->mant, x->mant);
        }
        //size_t pos;
        //size_t i;
        for(i = midlength; i >= 0; i--)
        {
            if(x->mant->limbs[i] > 0)
            {
                break;
            }
        }
        pos = APINT_LIMB_BITS * (maxlength-i-1);
        int xsize = maxlength * APINT_LIMB_BITS;
        int bitpos = __builtin_clzll(x->mant->limbs[i]);
        pos = pos + bitpos;
        overflow = (xsize-pos);
        overflow = mid_pos_bitwise_val - overflow;
        int full_limbs_shifted = overflow / APINT_LIMB_BITS;
        overflow -= full_limbs_shifted * APINT_LIMB_BITS;
        int full_limbs_shifted_1 = full_limbs_shifted + 1;
        for (int i = maxlength - 1; i >= full_limbs_shifted - 1; i -= 2)
        {
            x->mant->limbs[i] = x->mant->limbs[i - full_limbs_shifted];
            x->mant->limbs[i - 1] = x->mant->limbs[i - full_limbs_shifted_1];
        }
        if (overflow) {
            int rightshiftamt = (APINT_LIMB_BITS - overflow);
            for (int i = maxlength - 1; i > 0; i--) {
                x->mant->limbs[i] = (x->mant->limbs[i] << overflow) + (x->mant->limbs[i - 1] >> rightshiftamt);
            }
            x->mant->limbs[0] <<= overflow;
        }
        x->exp -= (apfp_exp_t) overflow;
    }
    return is_exact;
}

// Unrolling
// added ILP
bool apfp_add(apfp_ptr x, apfp_srcptr a, apfp_srcptr b)
{
    assert(x->mant->length == a->mant->length);
    assert(x->mant->length == b->mant->length);

    bool is_exact = true;
    // After swap, `a` is guaranteed to have largest exponent
    if (b->exp > a->exp){
        apfp_srcptr t = a; a = b; b = t;
    }

    // Align `b` mantissa to `a` given exponent difference
    apfp_exp_t factor = a->exp - b->exp;
    int maxlength = b->mant->length;
    int midlength = (b->mant->length/2)+1;
    for (int i = 0; i < midlength; i+=4){
        x->mant->limbs[i] = b->mant->limbs[i];
        x->mant->limbs[i+1] = b->mant->limbs[i+1];
        x->mant->limbs[i+2] = b->mant->limbs[i+2];
        x->mant->limbs[i+3] = b->mant->limbs[i+3];
    }
    x->mant->sign = b->mant->sign;

    int full_limbs_shifted = factor / APINT_LIMB_BITS;
    factor -= full_limbs_shifted * APINT_LIMB_BITS;

    int full_limbs_shifted_1 = full_limbs_shifted-1;
    // right shift
    for (int i = full_limbs_shifted; i  < maxlength; i+=2){
        x->mant->limbs[i-full_limbs_shifted] = x->mant->limbs[i];
        x->mant->limbs[i-full_limbs_shifted_1] = x->mant->limbs[i+1];
    }
    if (factor){
        int leftshiftamt = (APINT_LIMB_BITS - factor);
        for (int i = 0; i < midlength; ++i){
            x->mant->limbs[i] = (x->mant->limbs[i] >> factor) + (x->mant->limbs[i + 1] << leftshiftamt);
        }
    }
    size_t pos;
    size_t i;
    size_t overflow;

    int mid_pos_bitwise_val = MID_POS_BITWISE(x);
    // Add mantissa, shift by carry and update exponent
    x->exp = a->exp;
    unsigned char carry1 = 0;
    unsigned char carry2 = 0;
    unsigned char carry3 = 0;
    unsigned char carry4 = 0;
    unsigned char borrow1 = 0;
    unsigned char borrow2 = 0;
    unsigned char borrow3 = 0;
    unsigned char borrow4 = 0;

    int is_greater1,is_greater2,is_greater3,is_greater4 ;
    int is_greater=0;
    if (x->mant->sign == a->mant->sign)
    {
        for (apint_size_t i = 0; i < midlength; i+=4)
        {
            carry1 = _addcarryx_u64(carry4, x->mant->limbs[i], a->mant->limbs[i], &x->mant->limbs[i]);
            carry2 = _addcarryx_u64(carry1, x->mant->limbs[i+1], a->mant->limbs[i+1], &x->mant->limbs[i+1]);
            carry3 = _addcarryx_u64(carry2, x->mant->limbs[i+2], a->mant->limbs[i+2], &x->mant->limbs[i+2]);
            carry4 = _addcarryx_u64(carry3, x->mant->limbs[i+3], a->mant->limbs[i+3], &x->mant->limbs[i+3]);
        }

        x->mant->sign = a->mant->sign;
        for(i = midlength; i >= 0; i--)
        {
            if(x->mant->limbs[i] > 0)
            {
                break;
            }
        }
        pos = APINT_LIMB_BITS * (maxlength-i-1);
        int xsize = maxlength * APINT_LIMB_BITS;
        int bitpos = __builtin_clzll(x->mant->limbs[i]);
        pos = pos + bitpos;
        overflow = (xsize-pos);
        if (MIDDLE_LEFT(x))
        {
            overflow -= mid_pos_bitwise_val;
            int leftshiftamt = (APINT_LIMB_BITS - overflow);
            for (int i = 0; i < midlength; i+=4)
            {
                x->mant->limbs[i] = (x->mant->limbs[i] >> overflow) + (x->mant->limbs[i + 1] << leftshiftamt);
                x->mant->limbs[i+1] = (x->mant->limbs[i+1] >> overflow) + (x->mant->limbs[i + 2] << leftshiftamt);
                x->mant->limbs[i+2] = (x->mant->limbs[i+2] >> overflow) + (x->mant->limbs[i + 3] << leftshiftamt);
                x->mant->limbs[i+3] = (x->mant->limbs[i+3] >> overflow) + (x->mant->limbs[i + 4] << leftshiftamt);
            }
            x->mant->limbs[x->mant->length - 1] >>= overflow;
            x->exp += (apfp_exp_t) overflow;
        }
        else if (overflow < mid_pos_bitwise_val)
        {
            overflow = mid_pos_bitwise_val - overflow;
            int rightshiftamt = (APINT_LIMB_BITS - overflow);
            for (int i = midlength ; i > 3; i-=4) {
                x->mant->limbs[i] = (x->mant->limbs[i] << overflow) + (x->mant->limbs[i - 1] >> rightshiftamt);
                x->mant->limbs[i-1] = (x->mant->limbs[i-1] << overflow) + (x->mant->limbs[i - 2] >> rightshiftamt);
                x->mant->limbs[i-2] = (x->mant->limbs[i-2] << overflow) + (x->mant->limbs[i - 3] >> rightshiftamt);
                x->mant->limbs[i-3] = (x->mant->limbs[i-3] << overflow) + (x->mant->limbs[i - 4] >> rightshiftamt);
            }
            x->mant->limbs[0] <<= overflow;
            x->exp -= (apfp_exp_t) overflow;
        }
    }
    else{
        //TODO: Probably some bug here. This if condition shouldn't be required
        if (x->mant->sign == 1)
        {
            //apint_minus(x->mant, x->mant, a->mant);
            for (int i = midlength; i >= 0; i-=4)
            {
                is_greater1=(a->mant->limbs[i] > x->mant->limbs[i]);
                is_greater2=(a->mant->limbs[i-1] > x->mant->limbs[i-1]);
                is_greater3=(a->mant->limbs[i-2] > x->mant->limbs[i-2]);
                is_greater4=(a->mant->limbs[i-3] > x->mant->limbs[i-3]);
                is_greater = is_greater | is_greater1|is_greater2|is_greater3|is_greater4;
            }
            if (is_greater) // a > b so a-b
            {
                x->mant->sign = a->mant->sign;
                for (apint_size_t i = 0; i < a->mant->length; i+=4)
                {
                    borrow1 = _subborrow_u64(borrow4, a->mant->limbs[i], x->mant->limbs[i], &x->mant->limbs[i]);
                    borrow2 = _subborrow_u64(borrow1, a->mant->limbs[i+1], x->mant->limbs[i+1], &x->mant->limbs[i+1]);
                    borrow3 = _subborrow_u64(borrow2, a->mant->limbs[i+2], x->mant->limbs[i+2], &x->mant->limbs[i+2]);
                    borrow4 = _subborrow_u64(borrow3, a->mant->limbs[i+3], x->mant->limbs[i+3], &x->mant->limbs[i+3]);
                }
            }
            else // b > a so -(b-a)
            {
                x->mant->sign = -a->mant->sign;
                for (apint_size_t i = 0; i < a->mant->length; i++)
                {
                    borrow1 = _subborrow_u64(borrow4, x->mant->limbs[i], x->mant->limbs[i], &x->mant->limbs[i]);
                    borrow2 = _subborrow_u64(borrow1, x->mant->limbs[i+1], x->mant->limbs[i+1], &x->mant->limbs[i+1]);
                    borrow3 = _subborrow_u64(borrow2, x->mant->limbs[i+2], x->mant->limbs[i+2], &x->mant->limbs[i+2]);
                    borrow4 = _subborrow_u64(borrow3, x->mant->limbs[i+3], x->mant->limbs[i+3], &x->mant->limbs[i+3]);
                }
            }
        }
        else{
            apint_minus(x->mant, a->mant, x->mant);

        }
        size_t pos;
        size_t i;
        for(i = midlength; i >= 0; i--)
        {
            if(x->mant->limbs[i] > 0)
            {
                break;
            }
        }
        pos = APINT_LIMB_BITS * (maxlength-i-1);
        int xsize = maxlength * APINT_LIMB_BITS;
        int bitpos = __builtin_clzll(x->mant->limbs[i]);
        pos = pos + bitpos;
        overflow = (xsize-pos);
        overflow = mid_pos_bitwise_val - overflow;
        int full_limbs_shifted = overflow / APINT_LIMB_BITS;
        overflow -= full_limbs_shifted * APINT_LIMB_BITS;
        int full_limbs_shifted_1 = full_limbs_shifted + 1;
        for (int i = maxlength - 1; i >= full_limbs_shifted - 1; i -= 2)
        {
            x->mant->limbs[i] = x->mant->limbs[i - full_limbs_shifted];
            x->mant->limbs[i - 1] = x->mant->limbs[i - full_limbs_shifted_1];
        }
        if (overflow) {
            int rightshiftamt = (APINT_LIMB_BITS - overflow);
            for (int i = maxlength - 1; i > 0; i--) {
                x->mant->limbs[i] = (x->mant->limbs[i] << overflow) + (x->mant->limbs[i - 1] >> rightshiftamt);
            }
            x->mant->limbs[0] <<= overflow;
        }
        x->exp -= (apfp_exp_t) overflow;
    }
    return is_exact;
}

//a-b
bool apfp_sub_base(apfp_ptr x, apfp_srcptr a, apfp_srcptr b)
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
    apint_sub(x->mant, a->mant, x->mant); //x->mant->sign is set here
    if(swapped)
    {
            x->mant->sign = -x->mant->sign;
    }
    x->exp = a->exp;
    if(MIDDLE_LEFT(x) !=0 && (apint_getlimb(x->mant, 0) & 0x1ull) != 0)
        is_exact = false;

    adjust_alignment(x);
    return is_exact;
}

//Optimization 1
bool apfp_sub_optim1(apfp_ptr x, apfp_srcptr a, apfp_srcptr b)
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
    for (int i = 0; i < b->mant->length; i+=4)
    {
        x->mant->limbs[i] = b->mant->limbs[i];
        x->mant->limbs[i+1] = b->mant->limbs[i+1];
        x->mant->limbs[i+2] = b->mant->limbs[i+2];
        x->mant->limbs[i+3] = b->mant->limbs[i+3];
    }
    x->mant->sign = b->mant->sign;
    int midlength = (b->mant->length/2)+1;
    int maxlength = b->mant->length;

    if(factor)
    {
        int full_limbs_shifted = factor / APINT_LIMB_BITS;
        factor -= full_limbs_shifted * APINT_LIMB_BITS;

        int full_limbs_shifted_1 = full_limbs_shifted-1;
        for (int i = full_limbs_shifted; i  < x->mant->length-1; i+=2)
        {
            x->mant->limbs[i-full_limbs_shifted] = x->mant->limbs[i];
            x->mant->limbs[i-full_limbs_shifted_1] = x->mant->limbs[i+1];
        }

        if (factor)
        {
            int leftshiftamt = (APINT_LIMB_BITS - factor);
            for (int i = 0; i < x->mant->length - 1; ++i)
            {
                x->mant->limbs[i] = (x->mant->limbs[i] >> factor) + (x->mant->limbs[i + 1] << leftshiftamt);
            }
            x->mant->limbs[x->mant->length - 1] >>= factor;
        }
    }
    unsigned char carry1 = 0;
    unsigned char carry2 = 0;
    unsigned char carry3 = 0;
    unsigned char carry4 = 0;
    if(x->mant->sign == a->mant->sign)
    {
        apint_minus(x->mant, a->mant, x->mant); //sign is set here
    }
    else
    {
        for (apint_size_t i = 0; i < midlength; i+=4)
        {
            carry1 = _addcarryx_u64(carry4, x->mant->limbs[i], a->mant->limbs[i], &x->mant->limbs[i]);
            carry2 = _addcarryx_u64(carry1, x->mant->limbs[i+1], a->mant->limbs[i+1], &x->mant->limbs[i+1]);
            carry3 = _addcarryx_u64(carry2, x->mant->limbs[i+2], a->mant->limbs[i+2], &x->mant->limbs[i+2]);
            carry4 = _addcarryx_u64(carry3, x->mant->limbs[i+3], a->mant->limbs[i+3], &x->mant->limbs[i+3]);
        }

        x->mant->sign = a->mant->sign;
    }

    if(swapped)
    {
        x->mant->sign = -x->mant->sign;
    }
    x->exp = a->exp;
    if(MIDDLE_LEFT(x) !=0 && (apint_getlimb(x->mant, 0) & 0x1ull) != 0)
        is_exact = false;

    adjust_alignment(x);
    return is_exact;
}

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
    for (int i = 0; i < b->mant->length; i+=4)
    {
        x->mant->limbs[i] = b->mant->limbs[i];
        x->mant->limbs[i+1] = b->mant->limbs[i+1];
        x->mant->limbs[i+2] = b->mant->limbs[i+2];
        x->mant->limbs[i+3] = b->mant->limbs[i+3];
    }
    x->mant->sign = b->mant->sign;
    int midlength = (b->mant->length/2)+1;
    int maxlength = b->mant->length;
    size_t pos;
    size_t i;
    size_t overflow;

    int mid_pos_bitwise_val = MID_POS_BITWISE(x);
    if(factor)
    {
        int full_limbs_shifted = factor / APINT_LIMB_BITS;
        factor -= full_limbs_shifted * APINT_LIMB_BITS;

        int full_limbs_shifted_1 = full_limbs_shifted-1;
        for (int i = full_limbs_shifted; i  < x->mant->length-1; i+=2)
        {
            x->mant->limbs[i-full_limbs_shifted] = x->mant->limbs[i];
            x->mant->limbs[i-full_limbs_shifted_1] = x->mant->limbs[i+1];
        }

        if (factor)
        {
            int leftshiftamt = (APINT_LIMB_BITS - factor);
            for (int i = 0; i < x->mant->length - 1; ++i)
            {
                x->mant->limbs[i] = (x->mant->limbs[i] >> factor) + (x->mant->limbs[i + 1] << leftshiftamt);
            }
            x->mant->limbs[x->mant->length - 1] >>= factor;
        }
    }
    unsigned char carry1 = 0;
    unsigned char carry2 = 0;
    unsigned char carry3 = 0;
    unsigned char carry4 = 0;
    x->exp = a->exp;
    if(x->mant->sign == a->mant->sign)
    {
        apint_minus(x->mant, a->mant, x->mant); //sign is set here
        for(i = midlength; i >= 0; i--)
        {
            if(x->mant->limbs[i] > 0)
            {
                break;
            }
        }
        pos = APINT_LIMB_BITS * (maxlength-i-1);
        int xsize = maxlength * APINT_LIMB_BITS;
        int bitpos = __builtin_clzll(x->mant->limbs[i]);
        pos = pos + bitpos;
        overflow = (xsize-pos);
        overflow = mid_pos_bitwise_val - overflow;
        int full_limbs_shifted = overflow / APINT_LIMB_BITS;
        overflow -= full_limbs_shifted * APINT_LIMB_BITS;
        int full_limbs_shifted_1 = full_limbs_shifted + 1;
        for (i = maxlength - 1; i >= full_limbs_shifted - 1; i -= 2)
        {
            x->mant->limbs[i] = x->mant->limbs[i - full_limbs_shifted];
            x->mant->limbs[i - 1] = x->mant->limbs[i - full_limbs_shifted_1];
        }
        if (overflow) {
            int rightshiftamt = (APINT_LIMB_BITS - overflow);
            for ( i = maxlength - 1; i > 0; i--) {
                x->mant->limbs[i] = (x->mant->limbs[i] << overflow) + (x->mant->limbs[i - 1] >> rightshiftamt);
            }
            x->mant->limbs[0] <<= overflow;
        }
        x->exp -= (apfp_exp_t) overflow;
    }
    else
    {
        for (apint_size_t i = 0; i < midlength; i+=4)
        {
            carry1 = _addcarryx_u64(carry4, x->mant->limbs[i], a->mant->limbs[i], &x->mant->limbs[i]);
            carry2 = _addcarryx_u64(carry1, x->mant->limbs[i+1], a->mant->limbs[i+1], &x->mant->limbs[i+1]);
            carry3 = _addcarryx_u64(carry2, x->mant->limbs[i+2], a->mant->limbs[i+2], &x->mant->limbs[i+2]);
            carry4 = _addcarryx_u64(carry3, x->mant->limbs[i+3], a->mant->limbs[i+3], &x->mant->limbs[i+3]);
        }
        for(i = midlength; i >= 0; i--)
        {
            if(x->mant->limbs[i] > 0)
            {
                break;
            }
        }
        pos = APINT_LIMB_BITS * (maxlength-i-1);
        int xsize = maxlength * APINT_LIMB_BITS;
        int bitpos = __builtin_clzll(x->mant->limbs[i]);
        pos = pos + bitpos;
        overflow = (xsize-pos);
        if (overflow > mid_pos_bitwise_val)
        {
            overflow -= mid_pos_bitwise_val;
            int leftshiftamt = (APINT_LIMB_BITS - overflow);
            for (int i = 0; i < midlength; ++i)
            {
                x->mant->limbs[i] = (x->mant->limbs[i] >> overflow) + (x->mant->limbs[i + 1] << leftshiftamt);
            }
            x->mant->limbs[x->mant->length - 1] >>= overflow;
            x->exp += (apfp_exp_t) overflow;
        }
        else if (overflow < mid_pos_bitwise_val)
        {
            overflow = mid_pos_bitwise_val - overflow;
            int rightshiftamt = (APINT_LIMB_BITS - overflow);
            for (i = midlength ; i > 0; i--) {
                x->mant->limbs[i] = (x->mant->limbs[i] << overflow) + (x->mant->limbs[i - 1] >> rightshiftamt);
            }
            x->mant->limbs[0] <<= overflow;
            x->exp -= (apfp_exp_t) overflow;
        }
        x->mant->sign = a->mant->sign;
    }

    if(swapped)
    {
        x->mant->sign = -x->mant->sign;
    }
    if(MIDDLE_LEFT(x) !=0 && (apint_getlimb(x->mant, 0) & 0x1ull) != 0)
        is_exact = false;

    //adjust_alignment(x);
    return is_exact;
}

//a-b
//optimization 1. Merged to 1.
bool apfp_sub_segfault(apfp_ptr x, apfp_srcptr a, apfp_srcptr b)
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
    //apint_copy(x->mant, b->mant);
    for (int i = 0; i < b->mant->length; i+=4)
    {
        x->mant->limbs[i] = b->mant->limbs[i];
        x->mant->limbs[i+1] = b->mant->limbs[i+1];
        x->mant->limbs[i+2] = b->mant->limbs[i+2];
        x->mant->limbs[i+3] = b->mant->limbs[i+3];
    }
    x->mant->sign = b->mant->sign;

    if(factor)
    {
        int full_limbs_shifted = factor / APINT_LIMB_BITS;
        factor -= full_limbs_shifted * APINT_LIMB_BITS;

        int full_limbs_shifted_1 = full_limbs_shifted-1;
        for (int i = full_limbs_shifted; i  < x->mant->length; i+=2)
        {
            x->mant->limbs[i-full_limbs_shifted] = x->mant->limbs[i];
            x->mant->limbs[i-full_limbs_shifted_1] = x->mant->limbs[i+1];
        }

        if (factor)
        {
            int leftshiftamt = (APINT_LIMB_BITS - factor);
            for (int i = 0; i < x->mant->length - 1; ++i)
            {
                x->mant->limbs[i] = (x->mant->limbs[i] >> factor) + (x->mant->limbs[i + 1] << leftshiftamt);
            }
            x->mant->limbs[x->mant->length - 1] >>= factor;
        }
    }

    unsigned char carry1 = 0;
    unsigned char carry2 = 0;
    unsigned char carry3 = 0;
    unsigned char carry4 = 0;
    int midlength = (b->mant->length/2)+1;

    unsigned char borrow1 = 0;
    unsigned char borrow2 = 0;
    unsigned char borrow3 = 0;
    unsigned char borrow4 = 0;

    int is_greater1,is_greater2,is_greater3,is_greater4 ;
    int is_greater=0;
    if(x->mant->sign == a->mant->sign)
    {
        for (int i = midlength; i >= 0; i-=4)
        {
            is_greater1=(a->mant->limbs[i] > x->mant->limbs[i]);
            is_greater2=(a->mant->limbs[i-1] > x->mant->limbs[i-1]);
            is_greater3=(a->mant->limbs[i-2] > x->mant->limbs[i-2]);
            is_greater4=(a->mant->limbs[i-3] > x->mant->limbs[i-3]);
            is_greater = is_greater | is_greater1|is_greater2|is_greater3|is_greater4;
        }
        if (is_greater) // a > b so a-b
        {
            x->mant->sign = a->mant->sign;
            for (apint_size_t i = 0; i < a->mant->length; i+=4)
            {
                borrow1 = _subborrow_u64(borrow4, a->mant->limbs[i], x->mant->limbs[i], &x->mant->limbs[i]);
                borrow2 = _subborrow_u64(borrow1, a->mant->limbs[i+1], x->mant->limbs[i+1], &x->mant->limbs[i+1]);
                borrow3 = _subborrow_u64(borrow2, a->mant->limbs[i+2], x->mant->limbs[i+2], &x->mant->limbs[i+2]);
                borrow4 = _subborrow_u64(borrow3, a->mant->limbs[i+3], x->mant->limbs[i+3], &x->mant->limbs[i+3]);
            }
        }
        else // b > a so -(b-a)
        {
            x->mant->sign = -b->mant->sign;
            for (apint_size_t i = 0; i < a->mant->length; i++)
            {
                borrow1 = _subborrow_u64(borrow4, x->mant->limbs[i], x->mant->limbs[i], &x->mant->limbs[i]);
                borrow2 = _subborrow_u64(borrow1, x->mant->limbs[i+1], x->mant->limbs[i+1], &x->mant->limbs[i+1]);
                borrow3 = _subborrow_u64(borrow2, x->mant->limbs[i+2], x->mant->limbs[i+2], &x->mant->limbs[i+2]);
                borrow4 = _subborrow_u64(borrow3, x->mant->limbs[i+3], x->mant->limbs[i+3], &x->mant->limbs[i+3]);
            }
        }
    }
    else
    {
        for (apint_size_t i = 0; i < midlength; i+=4)
        {
            carry1 = _addcarryx_u64(carry4, x->mant->limbs[i], a->mant->limbs[i], &x->mant->limbs[i]);
            carry2 = _addcarryx_u64(carry1, x->mant->limbs[i+1], a->mant->limbs[i+1], &x->mant->limbs[i+1]);
            carry3 = _addcarryx_u64(carry2, x->mant->limbs[i+2], a->mant->limbs[i+2], &x->mant->limbs[i+2]);
            carry4 = _addcarryx_u64(carry3, x->mant->limbs[i+3], a->mant->limbs[i+3], &x->mant->limbs[i+3]);
        }
        x->mant->sign = a->mant->sign;
    }

    if(swapped)
    {
        x->mant->sign = -x->mant->sign;
    }
    x->exp = a->exp;
    //if(MIDDLE_LEFT(x) !=0 && (apint_getlimb(x->mant, 0) & 0x1ull) != 0)
    //    is_exact = false;

    adjust_alignment(x);
    return is_exact;
}

bool apfp_mul(apfp_ptr x, apfp_srcptr a, apfp_srcptr b)
{
    x->exp = a->exp + b->exp;
    apint_mul(x->mant, a->mant, b->mant);
    bool is_exact = adjust_alignment(x);

    if(a->mant->sign == b->mant->sign)
    {
        apfp_set_pos(x);
    }
    else
    {
        apfp_set_neg(x);
    }

    return is_exact;
}

bool apfp_mul_unroll(apfp_ptr x, apfp_srcptr a, apfp_srcptr b)
{
    x->exp = a->exp + b->exp;
    apint_mul_unroll(x->mant, a->mant, b->mant);
    bool is_exact = adjust_alignment(x);

    if(a->mant->sign == b->mant->sign)
    {
        apfp_set_pos(x);
    }
    else
    {
        apfp_set_neg(x);
    }

    return is_exact;
}
