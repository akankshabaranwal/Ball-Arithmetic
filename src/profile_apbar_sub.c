#include <apint.h>
#include <apfp.h>
#include <apbar.h>

int main()
{
    apbar_t apbar_test[3];
    apbar_init(apbar_test[0], 128);
    apbar_init(apbar_test[1], 128);
    apbar_init(apbar_test[2], 128);

    apbar_set_midpt_mant(apbar_test[0], 0, 0xC4C6628B80DC1CD1);
    apbar_set_midpt_mant(apbar_test[0], 1, 0xC90FDAA22168C234);
    apbar_set_midpt_exp(apbar_test[0], -126);
    apbar_set_rad(apbar_test[0], 536870912, -156);
    apbar_test[0]->midpt->mant->sign=1;

    apbar_set_midpt_mant(apbar_test[1], 0, 0xC4C6628B80DC1CD1);
    apbar_set_midpt_mant(apbar_test[1], 1, 0xC90FDAA22168C234);
    apbar_set_midpt_exp(apbar_test[1], -126);
    apbar_set_rad(apbar_test[1], 536870912, -156);
    apbar_test[1]->midpt->mant->sign=-1;

    apbar_print_msg("pi is:", apbar_test[0]);

    apbar_sub(apbar_test[2], apbar_test[1], apbar_test[0], 128);

    apbar_print_msg("pi - -pi is:", apbar_test[2]);

    apbar_free(apbar_test[0]);
    apbar_free(apbar_test[1]);
    apbar_free(apbar_test[2]);
    return 0;
}