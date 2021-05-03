#include <stdio.h>
#include "apbar.h"

int main(int argc, char const *argv[])
{
    apbar_t x, a, b;

    apbar_init(x, 128);
    apbar_init(a, 128);
    apbar_init(b, 128);

    // Initializing radius
    apbar_set_rad(a,0x8000000000000000,0);
    apbar_set_rad(b,0x8000000000000001,0);

    // Initialize midpt
    apbar_set_midpt_exp(a,0);
    apbar_set_midpt_mant(a, 0,0x8000000000000000);
    apbar_set_midpt_mant(a, 1,0x8000000000000001);

    apbar_set_midpt_exp(b,0);
    apbar_set_midpt_mant(b, 0,0x8000000000000000);
    apbar_set_midpt_mant(b, 1,0x8000000000000001);

    apbar_add(x, a, b, 128);

    //TODO: Add print for apbar
    //printf("0x%llx 0x%llx\n", apint_getlimb(x->mant, 1), apint_getlimb(x->mant, 0));

    //TODO: Radius memory is not being freed currently
    apbar_free(x);
    apbar_free(a);
    apbar_free(b);

    // This is kinda junk but I'm testing sub
    apint_add_test();
    apint_sub_test();
    apint_mult_test();
}

void apint_add_test()
{
    apint_t x, a, b;

    apint_init(x, 128);
    apint_init(a, 128);
    apint_init(b, 128);

    apint_setlimb(a, 0, 0x0000000000000001);
    apint_setlimb(a, 1, 0x0);

    apint_setlimb(b, 0, 0x0000000000000000);
    apint_setlimb(b, 1, 0x0);

    apint_add(x, a, b);
    // apint_shiftr(x, 2);

    printf("0x%llx 0x%llx\n", apint_getlimb(x, 1), apint_getlimb(x, 0));

    apint_free(x);
    apint_free(a);
    apint_free(b);
}

void apint_sub_test()
{
    apint_t x, a, b;

    apint_init(x, 128);
    apint_init(a, 128);
    apint_init(b, 128);

    apint_setlimb(a, 0, 0x8000000000000000);
    apint_setlimb(a, 1, 0x1);

    apint_setlimb(b, 0, 0x8000000000000000);
    apint_setlimb(b, 1, 0x1);

    apint_sub(x, a, b);
    apint_shiftr(x, 2);

    printf("0x%llx 0x%llx\n", apint_getlimb(x, 1), apint_getlimb(x, 0));

    apint_free(x);
    apint_free(a);
    apint_free(b);
}

void apint_mult_test()
{
    apint_t x, a, b;

    apint_init(x, 256);
    apint_init(a, 128);
    apint_init(b, 128);

    apint_setlimb(a, 0, 0x1);
    apint_setlimb(a, 1, 0x0);

    apint_setlimb(b, 0, 0x1);
    apint_setlimb(b, 1, 0x0);

    apint_mul(x, a, b);
    apint_shiftr(x, 2);

    printf("0x%llx 0x%llx\n", apint_getlimb(x, 1), apint_getlimb(x, 0));
    printf("0x%llx 0x%llx\n", apint_getlimb(x, 3), apint_getlimb(x, 2));

    apint_free(x);
    apint_free(a);
    apint_free(b);
}
