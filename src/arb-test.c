#include <arb.h>
#include <tsc_x86.h>
#include <stdint.h>

int main()
{
    arb_t a, b, c;
    arb_init(a); arb_set_d(a, 10.333);
    arb_print(a); printf("\n");

    arb_init(b); arb_set_d(b, 3.11256);
    arb_print(b); printf("\n");

    arb_init(c);

    arb_add(c, a, b, 64);
    arb_print(c); printf("\n");


    printf("PI test\n");
    arb_t x, y, z;
    arb_init(x); arb_init(y); arb_init(z);
    arb_set_ui(x, 3);       /* x = 3 */
    printf("3 is:\t\t"); arb_print(x); printf("\n");
    arb_const_pi(y, 128);   /* y = pi, to 128 bits */
    printf("pi is:\t\t"); arb_print(y); printf("\n");
    mag_print(arb_radref(y)); printf("\n");
    printf("pi rad exp is: "); fmpz_print(&MAG_EXP(arb_radref(y))); printf("\n");
    printf("pi rad man is: %ld\n", MAG_MAN(arb_radref(y)));
    arb_add(z, y, x, 128);   /* y = y - x, to 53 bits */
    printf("pi + 3 is:\t"); arb_print(z); printf("\n");

    arb_t double_pi;
    arb_init(double_pi);

    arb_add(double_pi, y, y, 128);
    printf("pi + pi is:\t"); arb_print(double_pi); printf("\n");
    arb_clear(x); arb_clear(y); arb_clear(z);

    return 0;
}
