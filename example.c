#include <stdio.h>

#include "apint.h"

int main(int argc, char const *argv[])
{
    apint_t x, a, b;

    apint_init(x, 128);
    apint_init(a, 128);
    apint_init(b, 128);

    apint_setlimb(a, 0, 0x8000000000000000);
    apint_setlimb(a, 1, 0x1);

    apint_setlimb(b, 0, 0x8000000000000000);
    apint_setlimb(b, 1, 0x1);

    apint_add(x, a, b);

    printf("0x%llx 0x%llx\n", apint_getlimb(x, 1), apint_getlimb(x, 0));

    apint_free(x);
    apint_free(a);
    apint_free(b);

    return 0;
}
