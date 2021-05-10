#include <stdio.h>

#include <apint.h>
#include <apfp.h>
#include <apbar.h>

int main(int argc, char const *argv[])
{
    apbar_t x, y, z;
    apbar_init(x, 64);
    apbar_set_midpt_mant(x, 0, 5816961868702417);
    apbar_set_midpt_exp(x, 49);
    apbar_set_rad(x, 0, 0);
    apbar_print(x);
    printf("\n");

    apbar_init(y, 64);
    apbar_set_midpt_mant(y, 0, 1752215507021039);
    apbar_set_midpt_exp(y, 49);
    apbar_set_rad(y, 0, 0);
    apbar_print(y);
    printf("\n");

    apbar_add(z, x, y, 64);
    apbar_print(z);
    printf("\n"); // This should print (118268396495679 * 2^-43) +/- (0)

    apbar_t c, a, b;

    apbar_init(c, 128);
    apbar_init(a, 128);
    apbar_init(b, 128);

    // Initializing radius
    apbar_set_rad(a, 0x8000000000000000, 0);
    apbar_set_rad(b, 0x8000000000000001, 0);

    // Initialize midpt
    apbar_set_midpt_exp(a, 0);
    apbar_set_midpt_mant(a, 0, 0x8000000000000000);
    apbar_set_midpt_mant(a, 1, 0x8000000000000001);

    apbar_set_midpt_exp(b, 0);
    apbar_set_midpt_mant(b, 0, 0x8000000000000000);
    apbar_set_midpt_mant(b, 1, 0x8000000000000001);

    apbar_add(c, a, b, 128);

    //TODO: Add print for apbar
    //printf("0x%llx 0x%llx\n", apint_getlimb(x->mant, 1), apint_getlimb(x->mant, 0));
    apbar_print(c);
    printf("\n");

    //TODO: Radius memory is not being freed currently
    apbar_free(c);
    apbar_free(a);
    apbar_free(b);

    // This is kinda junk but I'm testing sub
    apint_add_test();
    apint_sub_test();
    // apint_mult_test();
}

void apint_add_test()
{
    apint_t x, a, b;

    apint_init(x, 128);
    apint_init(a, 128);
    apint_init(b, 128);

    apint_setlimb(a, 0, 0x1);
    apint_setlimb(a, 1, 0x0);

    apint_setlimb(b, 0, 0x1);
    apint_setlimb(b, 1, 0x0);

    apint_add(x, a, b);

    printf("0x%llx 0x%llx\n", apint_getlimb(x, 1), apint_getlimb(x, 0));
    apint_print(x);
    printf("\n");

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

    apint_setlimb(a, 0, 0x0000000000000001);
    apint_setlimb(a, 1, 0x0);

    apint_setlimb(b, 0, 0x0000000000000001);
    apint_setlimb(b, 1, 0x0);

    apint_sub(x, a, b);

    printf("0x%llx 0x%llx\n", apint_getlimb(x, 1), apint_getlimb(x, 0));
    apint_print(x);
    printf("\n");

    apint_free(x);
    apint_free(a);
    apint_free(b);
}

void apint_mult_test()
{
    apint_t x, a, b;

    apint_init(x, 256); // the number of bits for x needs to be the sum of bits for a and b
    apint_init(a, 128);
    apint_init(b, 128);

    apint_setlimb(x, 0, 0);
    apint_setlimb(x, 1, 0);
    apint_setlimb(x, 2, 0);
    apint_setlimb(x, 3, 0);

    apint_setlimb(a, 0, 0x2);
    apint_setlimb(a, 1, 0x0);

    apint_setlimb(b, 0, 0x0);
    apint_setlimb(b, 1, 0x1);

    apint_mul(x, a, b);

    printf("0x%llx 0x%llx\n", apint_getlimb(x, 1), apint_getlimb(x, 0));
    printf("0x%llx 0x%llx\n", apint_getlimb(x, 3), apint_getlimb(x, 2));

    apint_free(x);
    apint_free(a);
    apint_free(b);
}

void apint_div_test()
{
    apint_t x, a, b;

    apint_init(x, 256);
    apint_init(a, 128);
    apint_init(b, 128);

    apint_setlimb(x, 0, 0);
    apint_setlimb(x, 1, 0);
    apint_setlimb(x, 2, 0);
    apint_setlimb(x, 3, 0);

    apint_setlimb(a, 0, 0x2);
    apint_setlimb(a, 1, 0x0);

    apint_setlimb(b, 0, 0x0);
    apint_setlimb(b, 1, 0x1);

    apint_div(x, a, b);

    printf("0x%llx 0x%llx\n", apint_getlimb(x, 1), apint_getlimb(x, 0));
    printf("0x%llx 0x%llx\n", apint_getlimb(x, 3), apint_getlimb(x, 2));
    apint_print(x);
    printf("\n");

    apint_free(x);
    apint_free(a);
    apint_free(b);
}
