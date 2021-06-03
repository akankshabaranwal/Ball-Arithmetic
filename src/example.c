#include <stdio.h>

#include <apint.h>
#include <apfp.h>
#include <apbar.h>

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

    // This is kinda jank but I'm testing some apint functions - Julia
    // apint_add_test();
    // apint_sub_test();
    apint_mult_test();
    // apint_add_karatsuba_test();
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

    apint_init(x, 128); // the number of bits for x needs to be the sum of real bits in a and b
    apint_init(a, 128);
    apint_init(b, 128);

    apint_setlimb(x, 0, 0);
    apint_setlimb(x, 1, 0);

    apint_setlimb(a, 0, 0x3);
    apint_setlimb(a, 1, 0x0); // 0x8 0x0 * 0x0 0x2 = 0x10 0x0

    apint_setlimb(b, 0, 0x2);
    apint_setlimb(b, 1, 0x0);

    apint_mul_karatsuba(x, a, b);
    // apint_mul(x, a, b);

    printf("0x%llx 0x%llx\n", apint_getlimb(x, 1), apint_getlimb(x, 0));
    printf("Final length of x: %d\n", x->length);

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

void apint_add_karatsuba_test()
{
    apint_t x, a, b;

    apint_init(x, 192);
    apint_init(a, 128);
    apint_init(b, 128);

    apint_setlimb(x, 0, 0);
    apint_setlimb(x, 1, 0);
    apint_setlimb(x, 2, 0);
    apint_setlimb(x, 3, 0);
    apint_setlimb(x, 4, 0);

    apint_setlimb(a, 0, 0x0);
    apint_setlimb(a, 1, 0x8000000000000000);

    apint_setlimb(b, 0, 0x0);
    apint_setlimb(b, 1, 0x8000000000000000);

    apint_add_karatsuba(x, a, b);

    printf("apint_add_karatsuba_test: 0x%llx 0x%llx 0x%llx\n", apint_getlimb(x, 2), apint_getlimb(x, 1), apint_getlimb(x, 0));
    printf("\n");

    apint_free(x);
    apint_free(a);
    apint_free(b);
}
