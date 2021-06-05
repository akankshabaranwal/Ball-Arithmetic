#include <stdio.h>

#include "apbar2.h"

const unsigned int prec = 128u;

int main(int argc, char const *argv[])
{
    apbar2_t x, a, b;

    apbar2_init(x, prec);
    apbar2_init(a, prec);
    apbar2_init(b, prec);

    apbar2_set_d(a, M_PI);
    apbar2_set_d(b, M_E);

    apbar2_add(x, a, b, prec);

    printf("a = ");
    apbar2_print(a);
    printf(" (a = %lf)\n", apbar2_get_d(a));

    printf("b = ");
    apbar2_print(b);
    printf(" (b = %lf)\n", apbar2_get_d(b));

    printf("x = a + b\n");

    printf("x = ");
    apbar2_print(x);
    printf(" (x = %lf)\n", apbar2_get_d(x));

    apbar2_free(x);
    apbar2_free(a);
    apbar2_free(b);

    return 0;
}
