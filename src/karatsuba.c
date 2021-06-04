#include <stdio.h>

#include <apint.h>
#include <apfp.h>
#include <apbar.h>

int main(int argc, char const *argv[])
{
    apint_mult_test();
}

void apint_mult_test()
{
    // Testing shifting

    apint_t x, a, b;

    apint_init(x, 128); // the number of bits for x needs to be the sum of bits for a and b
    apint_init(a, 128);
    apint_init(b, 128);

    apint_setlimb(x, 0, 0x0000);
    apint_setlimb(x, 1, 0x0000);

    apint_setlimb(a, 0, 0x3108);
    apint_setlimb(a, 1, 0x0000); // 0x8 0x0 * 0x0 0x2 = 0x10 0x0

    apint_setlimb(b, 0, 0x0032);
    apint_setlimb(b, 1, 0x0000); // if we have 0x1 0x0, what does this number actually represent???

    x->sign = 1;
    a->sign = 1;
    b->sign = 1;

    apint_mul_karatsuba(x, a, b);
    // apint_mul(x, a, b);

    printf("0x%llx 0x%llx\n", apint_getlimb(x, 1), apint_getlimb(x, 0));
    printf("Final length of x: %d\n", x->length);

    apint_free(x);
    apint_free(a);
    apint_free(b);
}