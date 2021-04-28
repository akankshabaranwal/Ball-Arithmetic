#include <stdio.h>

#include "apint.h"

int main(int argc, char const *argv[])
{
    uint64_t av[] = {0x8000000000000000, 0x1};
    uint64_t bv[] = {0x8000000000000000, 0x1};
    uint64_t dv[2];

    apint_t a = {
        .length = 2,
        .values = av
    };

    apint_t b = {
        .length = 2,
        .values = bv
    };

    apint_t dst = {
        .length = 2,
        .values = dv
    };

    apint_add(&dst, &a, &b);

    printf("%0xllx, 0x%llx\n", dst.values[1], dst.values[0]);

    return 0;
}
