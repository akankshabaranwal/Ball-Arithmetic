#include <apbar2.h>
#include <apbar.h>

static const int NUM_RUNS = 1000000;
static const int PRECISION = 32768;

void run_apbar_add_portable(apbar_t in1, apbar_t in2, apbar_t out)
{
    // Create separate functions for each precision so that they can be accordingly called.
    for(int i=0; i<NUM_RUNS; i++)
    {
        apbar_add_portable(out, in1, in2, PRECISION);
    }
}

void run_apbar2_add(apbar2_t in1, apbar2_t in2, apbar2_t out)
{
    // Create separate functions for each precision so that they can be accordingly called.
    for(int i=0; i<NUM_RUNS; i++)
    {
        apbar2_add(out, in1, in2, PRECISION);
    }
}

void run_apbar2_add_optim1(apbar2_t in1, apbar2_t in2, apbar2_t out)
{
    // Create separate functions for each precision so that they can be accordingly called.
    for(int i=0; i<NUM_RUNS; i++)
    {
        apbar2_add_optim1(out, in1, in2, PRECISION);
    }
}

void run_apbar_sub_portable(apbar_t in1, apbar_t in2, apbar_t out)
{
    // Create separate functions for each precision so that they can be accordingly called.
    for(int i=0; i<NUM_RUNS; i++)
    {
        apbar_sub_portable(out, in1, in2, PRECISION);
    }
}

void run_apbar2_sub(apbar2_t in1, apbar2_t in2, apbar2_t out)
{
    // Create separate functions for each precision so that they can be accordingly called.
    for(int i=0; i<NUM_RUNS; i++)
    {
        apbar2_sub(out, in1, in2, PRECISION);
    }
}

void run_apbar2_sub_optim1(apbar2_t in1, apbar2_t in2, apbar2_t out)
{
    // Create separate functions for each precision so that they can be accordingly called.
    for(int i=0; i<NUM_RUNS; i++)
    {
        apbar2_sub_optim1(out, in1, in2, PRECISION);
    }
}

int main()
{
    //Declare multiple precision apbar2 datatypes
    apbar2_t test_in1, test_in2, test_out;
    apbar_t ptest_in1, ptest_in2, ptest_out;

    test_in1->sign=0;
    test_in2->sign=0;

    apbar2_init(test_in1, PRECISION);
    apbar2_init(test_in2, PRECISION);
    apbar2_init(test_out, PRECISION);

    apbar2_set_d(test_in1, 3);
    apbar2_set_d(test_in2, 4);
    test_in1->midpt_exp = 1000;
    test_in2->midpt_exp = 1000;
    //test_in2->midpt_exp = PRECISION-APINT_LIMB_BITS;

    // Set sign
    ptest_in1->midpt->mant->sign=1;
    ptest_in2->midpt->mant->sign=1;

    apbar_init(ptest_in1, PRECISION);
    apbar_init(ptest_in2, PRECISION);
    apbar_init(ptest_out, PRECISION);

    apbar_set_d(ptest_in1, 3);
    apbar_set_d(ptest_in2, 4);

    ptest_in1->midpt->exp = 1000;
    ptest_in1->midpt->exp = 1000;
    //ptest_in2->midpt->exp = PRECISION-APINT_LIMB_BITS;

    //Addition
    //run_apbar_add_portable(ptest_in1, ptest_in2, ptest_out);
    //run_apbar2_add(test_in1, test_in2, test_out);
    //run_apbar2_add_optim1(test_in1, test_in2, test_out);

    //Subtraction
    //run_apbar_sub_portable(ptest_in1, ptest_in2, ptest_out);
    //run_apbar2_sub(test_in1, test_in2, test_out);
    //run_apbar2_sub_optim1(test_in1, test_in2, test_out);

    // Clean up memory
    apbar2_free(test_in1);
    apbar2_free(test_in2);
    apbar2_free(test_out);

    apbar_free(ptest_in1);
    apbar_free(ptest_in2);
    apbar_free(ptest_out);

    return 0;
}
