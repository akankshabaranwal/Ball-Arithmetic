#include <stdio.h>

#include <apint.h>
#include <apfp.h>
#include <apbar.h>

int main(int argc, char const *argv[])
{
    apbar_t x, y, z;
    apbar_init(x, 64);
    apbar_set_d(x, 10.333);
    apbar_print(x); printf("\n");

    apbar_init(y, 64);
    apbar_set_d(y, 3.11256);
    apbar_print(y); printf("\n");

    apbar_add(z, x, y, 64);
    apbar_print(z); printf("\n");

    apbar_free(x);
    apbar_free(y);
    apbar_free(z);

    printf("PI test\n");
    apbar_t pi, three, result;
    apbar_init(three, 128);
    apbar_set_d(three, 3.0);
    printf("3 is:\t\t"); apbar_print(three); printf("\n");
    apbar_init(pi, 128);
    apbar_set_midpt_mant(pi, 0, 0xC4C6628B80DC1CD1);
    apbar_set_midpt_mant(pi, 1, 0xC90FDAA22168C234);
    apbar_set_midpt_exp(pi, -126);
    apbar_set_rad(pi, 536870912, -156);
    printf("pi is:\t\t"); apbar_print(pi); printf("\n");

    apbar_add(result, three, pi, 128);
    printf("pi + 3 is:\t"); apbar_print(result); printf("\n");

    apbar_free(three);
    apbar_free(pi);

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
    apint_print(x); printf("\n");

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
    apint_print(x); printf("\n");

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
    apint_print(x); printf("\n");

    apint_free(x);
    apint_free(a);
    apint_free(b);
}
