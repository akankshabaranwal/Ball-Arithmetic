#include <stdio.h>

#include <apint.h>
#include <apfp.h>
#include <apbar.h>

int main(int argc, char const *argv[])
{
    apint_mult_test();
    apint_mult_test1();
}

void apint_mult_test()
{
    apint_t x, a, b;

    apint_init(x, 128); // the number of bits for x needs to be the sum of bits for a and b
    apint_init(a, 128);
    apint_init(b, 128);

    apint_setlimb(x, 0, 0x0000);
    apint_setlimb(x, 1, 0x0000);

    apint_setlimb(a, 0, 0x8108);
    apint_setlimb(a, 1, 0x0000); // 0x8 0x0 * 0x0 0x2 = 0x10 0x0

    apint_setlimb(b, 0, 0x8032);
    apint_setlimb(b, 1, 0x0000); // if we have 0x1 0x0, what does this number actually represent???

    x->sign = 1;
    a->sign = 1;
    b->sign = 1;

    // apint_mul_karatsuba(x, a, b);
    // apint_mul_karatsuba_extend_basecase(x, a, b);
    apint_mul_karatsuba_OPT1(x, a, b);
    // apint_mul(x, a, b);

    printf("0x%llx 0x%llx\n", apint_getlimb(x, 1), apint_getlimb(x, 0));
    printf("Final length of x: %d\n", x->length);

    apint_free(x);
    apint_free(a);
    apint_free(b);
}

void apint_mult_test1()
{
    apint_t x, a, b;

    apint_init(x, 1024); // the number of bits for x needs to be the sum of bits for a and b
    apint_init(a, 1024); // 16 limbs!
    apint_init(b, 1024);

    apint_setlimb(x, 0, 0x0000);
    apint_setlimb(x, 1, 0x0000);
    apint_setlimb(x, 2, 0x0000);
    apint_setlimb(x, 3, 0x0000);
    apint_setlimb(x, 4, 0x0000);
    apint_setlimb(x, 5, 0x0000);
    apint_setlimb(x, 6, 0x0000);
    apint_setlimb(x, 7, 0x0000);
    apint_setlimb(x, 8, 0x0000);
    apint_setlimb(x, 9, 0x0000);
    apint_setlimb(x, 10, 0x0000);
    apint_setlimb(x, 11, 0x0000);
    apint_setlimb(x, 12, 0x0000);
    apint_setlimb(x, 13, 0x0000);
    apint_setlimb(x, 14, 0x0000);
    apint_setlimb(x, 15, 0x0000);

    apint_setlimb(a, 0, 0xFFFFFFFFFFFFFFFF); // 0x8 0x0 * 0x0 0x2 = 0x10 0x0
    apint_setlimb(a, 1, 0x0);
    apint_setlimb(a, 2, 0x0);
    apint_setlimb(a, 3, 0x0);
    apint_setlimb(a, 4, 0x0);
    apint_setlimb(a, 5, 0x0);
    apint_setlimb(a, 6, 0x0000);
    apint_setlimb(a, 7, 0x0000);
    apint_setlimb(a, 8, 0x0000);
    apint_setlimb(a, 9, 0x0000);
    apint_setlimb(a, 10, 0x0000);
    apint_setlimb(a, 11, 0x0000);
    apint_setlimb(a, 12, 0x0000);
    apint_setlimb(a, 13, 0x0000);
    apint_setlimb(a, 14, 0x0000);
    apint_setlimb(a, 15, 0x0000);

    // if we have 0x1 0x0, what does this number actually represent???
    apint_setlimb(b, 0, 0xFFFFFFFFFFFFFFFF); // 0x8 0x0 * 0x0 0x2 = 0x10 0x0
    apint_setlimb(b, 1, 0x0);
    apint_setlimb(b, 2, 0x0);
    apint_setlimb(b, 3, 0x0);
    apint_setlimb(b, 4, 0x0);
    apint_setlimb(b, 5, 0x0);
    apint_setlimb(b, 6, 0x0);
    apint_setlimb(b, 7, 0x0);
    apint_setlimb(b, 8, 0x0000);
    apint_setlimb(b, 9, 0x0000);
    apint_setlimb(b, 10, 0x0000);
    apint_setlimb(b, 11, 0x0000);
    apint_setlimb(b, 12, 0x0000);
    apint_setlimb(b, 13, 0x0000);
    apint_setlimb(b, 14, 0x0000);
    apint_setlimb(b, 15, 0x0000);

    x->sign = 1;
    a->sign = 1;
    b->sign = 1;

    // apint_mul_karatsuba(x, a, b);
    // apint_mul_karatsuba_extend_basecase(x, a, b);
    // apint_mul_karatsuba_OPT1(x, a, b);
    apint_mul(x, a, b);

    for (int i = x->length - 1; i >= 0; i--)
    {
        printf("0x%llx ", apint_getlimb(x, i));
    }
    printf("\n");
    printf("Final length of x: %d\n", x->length);

    apint_free(x);
    apint_free(a);
    apint_free(b);
}