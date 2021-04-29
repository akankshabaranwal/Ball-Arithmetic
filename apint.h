#ifndef APINT_H
#define APINT_H

typedef unsigned long long  apint_limb_t;
typedef int                 apint_size_t;

#define APINT_LIMB_BYTES    sizeof(apint_limb_t)
#define APINT_LIMB_BITS     (APINT_LIMB_BYTES * 8)

typedef struct
{
    apint_limb_t *limbs;
    apint_size_t length;
} __apint_struct;

typedef __apint_struct          apint_t[1];
typedef __apint_struct          *apint_ptr;
typedef const __apint_struct    *apint_srcptr;

void apint_init(apint_t x, apint_size_t p);
void apint_free(apint_t x);
void apint_copy(apint_ptr dst, apint_srcptr src);

char apint_add(apint_ptr x, apint_srcptr a, apint_srcptr b);
void apint_shiftr(apint_ptr x, unsigned int shift);

inline apint_limb_t apint_getlimb(apint_ptr x, apint_size_t offset)
{
    return x->limbs[offset];
}

inline void apint_setlimb(apint_ptr x, apint_size_t offset, apint_limb_t limb)
{
    x->limbs[offset] = limb;
}

#endif /* !APINT_H */
