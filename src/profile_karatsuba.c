#include <apint.h>

static const int NUM_RUNS = 100000;
static const int PRECISION = 131072;

int main()
{
    apint_t x, a, b;

    apint_init(x, 128); // the number of bits for x needs to be the sum of bits for a and b
    apint_init(a, 128); // 16 limbs!
    apint_init(b, 128);

    apint_setlimb(x, 0, 0x0000);
    apint_setlimb(x, 1, 0x0000);
    // apint_setlimb(x, 2, 0x0000);
    // apint_setlimb(x, 3, 0x0000);
    // apint_setlimb(x, 4, 0x0000);
    // apint_setlimb(x, 5, 0x0000);
    // apint_setlimb(x, 6, 0x0000);
    // apint_setlimb(x, 7, 0x0000);
    // apint_setlimb(x, 8, 0x0000);
    // apint_setlimb(x, 9, 0x0000);
    // apint_setlimb(x, 10, 0x0000);
    // apint_setlimb(x, 11, 0x0000);
    // apint_setlimb(x, 12, 0x0000);
    // apint_setlimb(x, 13, 0x0000);
    // apint_setlimb(x, 14, 0x0000);
    // apint_setlimb(x, 15, 0x0000);

    apint_setlimb(a, 0, 0x3108); // 0x8 0x0 * 0x0 0x2 = 0x10 0x0
    apint_setlimb(a, 1, 0x0000);
    // apint_setlimb(a, 2, 0x3108);
    // apint_setlimb(a, 3, 0x0000);
    // apint_setlimb(a, 4, 0x3108);
    // apint_setlimb(a, 5, 0x0000);
    // apint_setlimb(a, 6, 0x3108);
    // apint_setlimb(a, 7, 0x0000);
    // apint_setlimb(a, 8, 0x0000);
    // apint_setlimb(a, 9, 0x0000);
    // apint_setlimb(a, 10, 0x0000);
    // apint_setlimb(a, 11, 0x0000);
    // apint_setlimb(a, 12, 0x0000);
    // apint_setlimb(a, 13, 0x0000);
    // apint_setlimb(a, 14, 0x0000);
    // apint_setlimb(a, 15, 0x0000);

    // if we have 0x1 0x0, what does this number actually represent???
    apint_setlimb(b, 0, 0x3108); // 0x8 0x0 * 0x0 0x2 = 0x10 0x0
    apint_setlimb(b, 1, 0x0000);
    // apint_setlimb(b, 2, 0x3108);
    // apint_setlimb(b, 3, 0x0000);
    // apint_setlimb(b, 4, 0x3108);
    // apint_setlimb(b, 5, 0x0000);
    // apint_setlimb(b, 6, 0x3108);
    // apint_setlimb(b, 7, 0x0000);
    // apint_setlimb(b, 8, 0x0000);
    // apint_setlimb(b, 9, 0x0000);
    // apint_setlimb(b, 10, 0x0000);
    // apint_setlimb(b, 11, 0x0000);
    // apint_setlimb(b, 12, 0x0000);
    // apint_setlimb(b, 13, 0x0000);
    // apint_setlimb(b, 14, 0x0000);
    // apint_setlimb(b, 15, 0x0000);

    x->sign = 1;
    a->sign = 1;
    b->sign = 1;

    for (int i = 0; i < NUM_RUNS; i++)
    {
        apint_mul_karatsuba(x, a, b);
    }

    apint_free(x);
    apint_free(a);
    apint_free(b);
    return 0;
}