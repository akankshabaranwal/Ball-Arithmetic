#ifndef APINT_H
#define APINT_H

#include <assert.h>
#include <immintrin.h>
#include <stdlib.h>
#include <string.h>
#include <flint/fmpz.h>

typedef unsigned long long apint_limb_t;
typedef uint apint_size_t;
typedef int sign_t;

#define APINT_LIMB_BYTES sizeof(apint_limb_t)
#define APINT_LIMB_BITS (APINT_LIMB_BYTES * 8)

typedef struct
{
    sign_t sign;
    apint_limb_t *limbs;
    apint_size_t length;
} __apint_struct;

typedef __apint_struct apint_t[1];
typedef __apint_struct *apint_ptr;
typedef const __apint_struct *apint_srcptr;

void apint_init(apint_t x, apint_size_t p);
void apint_print(apint_srcptr value);
void apint_free(apint_t x);
void apint_copy(apint_ptr dst, apint_srcptr src);
void apint_to_fmpz(fmpz_t res, apint_srcptr src);

static inline apint_limb_t apint_getlimb(apint_ptr x, apint_size_t offset);

void apint_add(apint_ptr x, apint_srcptr a, apint_srcptr b);
unsigned char apint_sub(apint_ptr x, apint_srcptr a, apint_srcptr b);
char apint_plus_portable(apint_ptr x, apint_srcptr a, apint_srcptr b);
unsigned char apint_plus(apint_ptr x, apint_srcptr a, apint_srcptr b);
unsigned char apint_minus(apint_ptr x, apint_srcptr a, apint_srcptr b);
int apint_is_greater(apint_srcptr a, apint_srcptr b);
void apint_mul_portable(apint_ptr x, apint_srcptr a, apint_srcptr b);
void apint_mul(apint_ptr x, apint_srcptr a, apint_srcptr b);
void apint_shiftr(apint_ptr x, unsigned int shift);
void apint_shiftl(apint_ptr x, unsigned int shift);

static inline void apint_setmsb(apint_ptr x)
{
    x->limbs[x->length - 1] |= 1ull << (APINT_LIMB_BITS - 1);
}

// To-do: Deprecate and remove this, we should have a better API for setting apint values.
static inline apint_limb_t apint_getlimb(apint_ptr x, apint_size_t offset)
{
    return x->limbs[offset];
}

// To-do: Deprecate and remove this, we should have a better API for setting apint values.
static inline void apint_setlimb(apint_ptr x, apint_size_t offset, apint_limb_t limb)
{
    x->limbs[offset] = limb;
}

void apint_add_test();
void apint_sub_test();
void apint_mult_test();

#endif /* !APINT_H */
