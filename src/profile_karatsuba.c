#include <apint.h>

static const int NUM_RUNS = 100000;
static const int PRECISION = 131072;

int main()
{
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

    for (int i = 0; i < NUM_RUNS; i++)
    {
        apint_mul_karatsuba_extend_basecase(x, a, b);
    }

    apint_free(x);
    apint_free(a);
    apint_free(b);
    return 0;
}