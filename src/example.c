#include <stdio.h>

#include "apbar.h"

const unsigned int prec = 128u;

int main(int argc, char const *argv[])
{
    apbar_t x, a, b;

    apbar_init(x, prec);
    apbar_init(a, prec);
    apbar_init(b, prec);

    apbar_set_d(a, M_PI);
    apbar_set_d(b, M_E);

    apbar_add(x, a, b, prec);

    printf("a = ");
    apbar_print(a);
    printf(" (a = %lf)\n", apbar_get_d(a));

    printf("b = ");
    apbar_print(b);
    printf(" (b = %lf)\n", apbar_get_d(b));

    printf("x = a + b\n");

    printf("x = ");
    apbar_print(x);
    printf(" (x = %lf)\n", apbar_get_d(x));

    apbar_free(x);
    apbar_free(a);
    apbar_free(b);

    return 0;
}
