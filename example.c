#include <stdio.h>

#include "apint.h"
#include "apfp.h"

int main(int argc, char const *argv[])
{
    apfp_t x, a, b;

    apint_init(x->mant, 128);
    apint_init(a->mant, 128);
    apint_init(b->mant, 128);

    a->exp = 0;
    apint_setlimb(a->mant, 0, 0x8000000000000000);
    apint_setlimb(a->mant, 1, 0x8000000000000001);

    b->exp = 0;
    apint_setlimb(b->mant, 0, 0x8000000000000000);
    apint_setlimb(b->mant, 1, 0x8000000000000001);

    apfp_add(x, a, b);

    printf("0x%llx 0x%llx\n", apint_getlimb(x->mant, 1), apint_getlimb(x->mant, 0));

    apint_free(x->mant);
    apint_free(a->mant);
    apint_free(b->mant);

    return 0;
}
