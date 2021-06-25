#ifndef APINT_H
#define APINT_H

#include <assert.h>
#include <x86intrin.h>
#include <stdlib.h>
#include <string.h>
#include <flint/fmpz.h>
#include <stdint.h>
#include <stdbool.h>

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
void apint_print_msg(const char *msg, apint_srcptr value);
void apint_free(apint_t x);
void apint_copy(apint_ptr dst, apint_srcptr src);
void apint_to_fmpz(fmpz_t res, apint_srcptr src);

static inline apint_limb_t apint_getlimb(apint_srcptr x, apint_size_t offset);

unsigned char apint_add_portable(apint_ptr x, apint_srcptr a, apint_srcptr b);
unsigned char apint_add(apint_ptr x, apint_srcptr a, apint_srcptr b);
unsigned char apint_add_plus(apint_ptr x, apint_srcptr a, apint_srcptr b);
unsigned char apint_sub_portable(apint_ptr x, apint_srcptr a, apint_srcptr b);
unsigned char apint_sub(apint_ptr x, apint_srcptr a, apint_srcptr b);
unsigned char apint_sub_minus(apint_ptr x, apint_srcptr a, apint_srcptr b);
char apint_plus_portable(apint_ptr x, apint_srcptr a, apint_srcptr b);
unsigned char apint_plus(apint_ptr x, apint_srcptr a, apint_srcptr b);
unsigned char apint_plus_optim1(apint_ptr x, apint_srcptr a, apint_srcptr b);
unsigned char apint_minus_portable(apint_ptr x, apint_srcptr a, apint_srcptr b);
unsigned char apint_minus(apint_ptr x, apint_srcptr a, apint_srcptr b);
unsigned char apint_minus_optim1(apint_ptr x, apint_srcptr a, apint_srcptr b);
int apint_is_greater(apint_srcptr a, apint_srcptr b);
void apint_mul_portable(apint_ptr x, apint_srcptr a, apint_srcptr b);
int apint_mul(apint_ptr x, apint_srcptr a, apint_srcptr b);
int apint_mul_unroll(apint_ptr x, apint_srcptr a, apint_srcptr b);

// apint_mul optimization 1 methods: unrolling
int apint_mul_OPT1(apint_ptr x, apint_srcptr a, apint_srcptr b);
int apint_mul_unroll12(apint_ptr x, apint_srcptr a, apint_srcptr b);

// Karatsuba helper methods
char apint_add_karatsuba(apint_ptr x, apint_srcptr a, apint_srcptr b);
unsigned long long apint_mul_karatsuba_base_case(apint_ptr x, apint_srcptr a, apint_srcptr b);

// Karatsuba main methods
uint64_t apint_mul_karatsuba(apint_ptr x, apint_srcptr a, apint_srcptr b);
uint64_t apint_mul_karatsuba_extend_basecase(apint_ptr x, apint_srcptr a, apint_srcptr b);

// Karatsuba recursive methods
uint64_t apint_mul_karatsuba_recurse(apint_ptr x, apint_srcptr a, apint_srcptr b);
uint64_t apint_mul_karatsuba_recurse_extend_basecase(apint_ptr x, apint_srcptr a, apint_srcptr b);

// Karatsuba optimization 1 methods; inlining functions
uint64_t apint_mul_karatsuba_OPT1(apint_ptr x, apint_srcptr a, apint_srcptr b);
uint64_t apint_mul_karatsuba_recurse_OPT1(apint_ptr x, apint_srcptr a, apint_srcptr b);

// Karatsuba optimization 2 methods; making sub calls faster
uint64_t apint_mul_karatsuba_OPT2(apint_ptr x, apint_srcptr a, apint_srcptr b);
uint64_t apint_mul_karatsuba_recurse_OPT2(apint_ptr x, apint_srcptr a, apint_srcptr b);

bool apint_shiftr(apint_ptr x, unsigned int shift);
bool apint_shiftr_optim1(apint_ptr x, unsigned int shift);
bool apint_shiftr_copy(apint_ptr dest, apint_srcptr src, unsigned int shift);
void apint_shiftl(apint_ptr x, unsigned int shift);
void apint_shiftl_optim1(apint_ptr x, unsigned int shift);
void apint_shiftl_optim2(apint_ptr x, unsigned int shift);
size_t apint_detectfirst1(apint_ptr x);
size_t apint_detectfirst1_optim1(apint_ptr x);

static inline void apint_setmsb(apint_ptr x)
{
    x->limbs[x->length - 1] |= 1llu << (APINT_LIMB_BITS - 1);
}

// To-do: Deprecate and remove this, we should have a better API for setting apint values.
static inline apint_limb_t apint_getlimb(apint_srcptr x, apint_size_t offset)
{
    return x->limbs[offset];
}

// To-do: Deprecate and remove this, we should have a better API for setting apint values.
static inline void apint_setlimb(apint_ptr x, apint_size_t offset, apint_limb_t limb)
{
    x->limbs[offset] = limb;
}

static inline void apint_trim(apint_ptr x, apint_ptr x_temp)
{
    for (int i = 0; i < x->length; i++) // least sig bits
    {
        x->limbs[i] = x_temp->limbs[i];
    }
}

// Find maximum between two numbers.
static inline int max(int num1, int num2)
{
    return (num1 > num2) ? num1 : num2;
}

// Find minimum between two numbers.
static inline int min(int num1, int num2)
{
    return (num1 > num2) ? num2 : num1;
}

// Fill apint with values from another apint.
static inline void apint_copyover(apint_ptr x_new, apint_ptr x_old, apint_size_t offset)
{
    for (int i = 0; i < x_new->length; i++)
    {
        x_new->limbs[i] = x_old->limbs[i + offset];
    }
}

void apint_mult_test();
void apint_mult_test1();

#endif /* !APINT_H */
