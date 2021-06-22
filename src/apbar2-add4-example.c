#include <stdio.h>

#include "apbar2.h"

const unsigned int prec = 256u;

int main(int argc, char const *argv[])
{
    apbar2_t x1, a1, b1;
    apbar2_t x2, a2, b2;
    apbar2_t x3, a3, b3;
    apbar2_t x4, a4, b4;

    apbar2_init(x1, prec); apbar2_init(a1, prec); apbar2_init(b1, prec);
    apbar2_init(x2, prec); apbar2_init(a2, prec); apbar2_init(b2, prec);
    apbar2_init(x3, prec); apbar2_init(a3, prec); apbar2_init(b3, prec);
    apbar2_init(x4, prec); apbar2_init(a4, prec); apbar2_init(b4, prec);

    apbar2_set_d(a1, M_PI);
    apbar2_set_d(b1, M_E);

    apbar2_add4(x1, a1, b1, x2, a2, b2, x3, a3, b3, x4, a4, b4, prec);

    printf("a = ");
    apbar2_print(a1);
    printf(" (a = %lf)\n", apbar2_get_d(a1));

    printf("b = ");
    apbar2_print(b1);
    printf(" (b = %lf)\n", apbar2_get_d(b1));

    printf("x = a + b\n");

    printf("x = ");
    apbar2_print(x1);
    printf(" (x = %lf)\n", apbar2_get_d(x1));

    apbar2_free(x1); apbar2_free(a1); apbar2_free(b1);
    apbar2_free(x2); apbar2_free(a2); apbar2_free(b2);
    apbar2_free(x3); apbar2_free(a3); apbar2_free(b3);
    apbar2_free(x4); apbar2_free(a4); apbar2_free(b4);

    return 0;
}
