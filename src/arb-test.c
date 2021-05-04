#include <arb.h>
#include <tsc_x86.h>

int main()
{
    slong prec;
    printf("radius exp size: %zu\n", sizeof(fmpz));
    printf("radius mantissa size: %zu\n", sizeof(mp_limb_t));

    arb_t x, y;
    arb_init(x); arb_init(y);
    myInt64 start = start_tsc();
    for (prec = 64; ; prec *= 2) {
        arb_const_pi(x, prec);
        arb_set_si(y, -10000);
        arb_exp(y, y, prec);
        arb_add(x, x, y, prec);
        arb_sin(y, x, prec);
        if (arb_rel_accuracy_bits(y) >= 53) break;
    }
    myInt64 cycles = stop_tsc(start);
    printf("Ran for %llu cycles\n", cycles);

    arb_printn(y, 15, 0); printf("\n");
    arb_printd(y, 15); printf("\n");
    arb_print(y); printf("\n");
    arb_clear(x); arb_clear(y);
    flint_cleanup();
    return 0;
}
