#include <arb.h>
#include <stdio.h>
#include <flint/flint.h>

#include "apbar.h"
#include "apbar2.h"
#include "tsc_x86.h"
#include "benchmark.h"

#define ITERATE(CODE) for (size_t i = 0; i < BENCHMARK_ITER; i++) CODE;

static double bench(benchmark_fun_t f, unsigned int prec)
{
    /* Warm up the cpu. */
    for (int i = 0; i < BENCHMARK_WARMUPS; ++i) {
        f(prec);
    }

    /* Start the logs. */
    double start = start_tsc();
    f(prec);
    double cycles = stop_tsc(start);

    return cycles;
}

// gyorgy: Added a wrapper for whatever we use for random limb generation so
//         that it's easier to change if we need to
static apint_limb_t urand()
{
    apint_limb_t ret_val;

    // Use the unix provided /dev/urandom file
    FILE *f = fopen("/dev/urandom", "r");
    fread(&ret_val, sizeof(ret_val), 1, f);
    fclose(f);
    return ret_val;
}

static arb_t arb_out, arb_in1, arb_in2;

void arblib_init(unsigned int prec)
{
    flint_rand_t rand;
    flint_randinit(rand);

    arb_init(arb_out);
    arb_init(arb_in1);
    arb_init(arb_in2);

    arb_randtest(arb_in1, rand, prec, 64);
    arb_randtest(arb_in2, rand, prec, 64);
}

void arblib_deinit(unsigned int prec)
{
    arb_clear(arb_out);
    arb_clear(arb_in1);
    arb_clear(arb_in2);
}

void arblib_add(unsigned int prec)
{
    for (size_t i = 0; i < BENCHMARK_ITER; i++)
    {
        arb_add(arb_out, arb_in1, arb_in2, prec);
        arb_add(arb_out, arb_in1, arb_in2, prec);
        arb_add(arb_out, arb_in1, arb_in2, prec);
        arb_add(arb_out, arb_in1, arb_in2, prec);
    }
}

void arblib_sub(unsigned int prec)
{
    for (size_t i = 0; i < BENCHMARK_ITER; i++)
    {
        arb_sub(arb_out, arb_in1, arb_in2, prec);
        arb_sub(arb_out, arb_in1, arb_in2, prec);
        arb_sub(arb_out, arb_in1, arb_in2, prec);
        arb_sub(arb_out, arb_in1, arb_in2, prec);
    }
}

void arblib_mul(unsigned int prec)
{
    for (size_t i = 0; i < BENCHMARK_ITER; i++)
    {
        arb_mul(arb_out, arb_in1, arb_in2, prec);
        arb_mul(arb_out, arb_in1, arb_in2, prec);
        arb_mul(arb_out, arb_in1, arb_in2, prec);
        arb_mul(arb_out, arb_in1, arb_in2, prec);
    }
}

static apbar_t apbar_out, apbar_in1, apbar_in2;

void barith_init(unsigned int prec)
{
    apbar_init(apbar_out, prec);
    apbar_init(apbar_in1, prec);
    apbar_init(apbar_in2, prec);
    apbar_in1->midpt->mant->sign = 1;
    apbar_in2->midpt->mant->sign = 1;

    size_t limbs = prec / APINT_LIMB_BITS;
    for (int i = 0; i < limbs; ++i) {
        apbar_set_midpt_mant(apbar_in1, i, urand());
        apbar_set_midpt_mant(apbar_in2, i, urand());
    }

    // I am setting the exponents of the midpoint so that the difference can't
    // be more than prec
    apfp_exp_t exp = random();
    apfp_exp_t diff = (apfp_exp_t) ((((double)random() / RAND_MAX) * 2 * prec) - prec);

    apbar_set_midpt_exp(apbar_in1, exp);
    apbar_set_midpt_exp(apbar_in2, exp + diff);

    // Same as above except with 64 not prec
    exp = random();
    diff = (apfp_exp_t) ((((double)random() / RAND_MAX) * 2 * 64) - 64);
    apbar_set_rad(apbar_in1, urand(), exp);
    apbar_set_rad(apbar_in2, urand(), exp + diff);
}

void barith_deinit(unsigned int prec)
{
    apbar_free(apbar_in1);
    apbar_free(apbar_in2);
    apbar_free(apbar_out);
}


//Keeping it for legacy purpose. Code with intrinsics
void barith_add(unsigned int prec)
{
    for (size_t i = 0; i < BENCHMARK_ITER; i++)
    {
        apbar_add(apbar_out, apbar_in1, apbar_in2, prec);
        apbar_add(apbar_out, apbar_in1, apbar_in2, prec);
        apbar_add(apbar_out, apbar_in1, apbar_in2, prec);
        apbar_add(apbar_out, apbar_in1, apbar_in2, prec);
    }
}

void barith_add_portable(unsigned int prec)
{
    for (size_t i = 0; i < BENCHMARK_ITER; i++)
    {
        apbar_add_portable(apbar_out, apbar_in1, apbar_in2, prec);
        apbar_add_portable(apbar_out, apbar_in1, apbar_in2, prec);
        apbar_add_portable(apbar_out, apbar_in1, apbar_in2, prec);
        apbar_add_portable(apbar_out, apbar_in1, apbar_in2, prec);
    }
}

//Name of apbar add functions are misleading. See comments in apfp.c codes for optimization details.
void barith_add_intrinsics(unsigned int prec)
{
    for (size_t i = 0; i < BENCHMARK_ITER; i++)
    {
        apbar_add(apbar_out, apbar_in1, apbar_in2, prec);
        apbar_add(apbar_out, apbar_in1, apbar_in2, prec);
        apbar_add(apbar_out, apbar_in1, apbar_in2, prec);
        apbar_add(apbar_out, apbar_in1, apbar_in2, prec);
    }
}

//Name of apbar add functions are misleading. See comments in apfp.c codes for optimization details.
void barith_add_midptrep(unsigned int prec)
{
    for (size_t i = 0; i < BENCHMARK_ITER; i++)
    {
        apbar_add_scalar(apbar_out, apbar_in1, apbar_in2, prec);
        apbar_add_scalar(apbar_out, apbar_in1, apbar_in2, prec);
        apbar_add_scalar(apbar_out, apbar_in1, apbar_in2, prec);
        apbar_add_scalar(apbar_out, apbar_in1, apbar_in2, prec);
    }
}

//Name of apbar add functions are misleading. See comments in apfp_add codes for optimization details.
void barith_add_nestedbranch(unsigned int prec)
{
    for (size_t i = 0; i < BENCHMARK_ITER; i++)
    {
        apbar_add_unroll(apbar_out, apbar_in1, apbar_in2, prec);
        apbar_add_unroll(apbar_out, apbar_in1, apbar_in2, prec);
        apbar_add_unroll(apbar_out, apbar_in1, apbar_in2, prec);
        apbar_add_unroll(apbar_out, apbar_in1, apbar_in2, prec);
    }
}

//Name of apbar add functions are misleading. See comments in apfp_add codes for optimization details.
void barith_add_norad_noexp(unsigned int prec)
{
    for (size_t i = 0; i < BENCHMARK_ITER; i++)
    {
        apbar_add_unroll_norad_noexp(apbar_out, apbar_in1, apbar_in2, prec);
        apbar_add_unroll_norad_noexp(apbar_out, apbar_in1, apbar_in2, prec);
        apbar_add_unroll_norad_noexp(apbar_out, apbar_in1, apbar_in2, prec);
        apbar_add_unroll_norad_noexp(apbar_out, apbar_in1, apbar_in2, prec);
    }
}

void barith_add_shiftr(unsigned int prec)
{
    for (size_t i = 0; i < BENCHMARK_ITER; i++)
    {
        apbar_add_shiftr(apbar_out, apbar_in1, apbar_in2, prec);
        apbar_add_shiftr(apbar_out, apbar_in1, apbar_in2, prec);
        apbar_add_shiftr(apbar_out, apbar_in1, apbar_in2, prec);
        apbar_add_shiftr(apbar_out, apbar_in1, apbar_in2, prec);
    }
}

void barith_add_shiftl(unsigned int prec)
{
    for (size_t i = 0; i < BENCHMARK_ITER; i++)
    {
        apbar_add_shiftl(apbar_out, apbar_in1, apbar_in2, prec);
        apbar_add_shiftl(apbar_out, apbar_in1, apbar_in2, prec);
        apbar_add_shiftl(apbar_out, apbar_in1, apbar_in2, prec);
        apbar_add_shiftl(apbar_out, apbar_in1, apbar_in2, prec);
    }
}

void barith_add_plus(unsigned int prec)
{
    for (size_t i = 0; i < BENCHMARK_ITER; i++)
    {
        apbar_add_plus(apbar_out, apbar_in1, apbar_in2, prec);
        apbar_add_plus(apbar_out, apbar_in1, apbar_in2, prec);
        apbar_add_plus(apbar_out, apbar_in1, apbar_in2, prec);
        apbar_add_plus(apbar_out, apbar_in1, apbar_in2, prec);
    }
}

void barith_add_detect1(unsigned int prec)
{
    for (size_t i = 0; i < BENCHMARK_ITER; i++)
    {
        apbar_add_detect1(apbar_out, apbar_in1, apbar_in2, prec);
        apbar_add_detect1(apbar_out, apbar_in1, apbar_in2, prec);
        apbar_add_detect1(apbar_out, apbar_in1, apbar_in2, prec);
        apbar_add_detect1(apbar_out, apbar_in1, apbar_in2, prec);
    }
}

void barith_add_merged(unsigned int prec)
{
    for (size_t i = 0; i < BENCHMARK_ITER; i++)
    {
        apbar_add_merged(apbar_out, apbar_in1, apbar_in2, prec);
        apbar_add_merged(apbar_out, apbar_in1, apbar_in2, prec);
        apbar_add_merged(apbar_out, apbar_in1, apbar_in2, prec);
        apbar_add_merged(apbar_out, apbar_in1, apbar_in2, prec);
    }
}

void barith_sub_portable(unsigned int prec)
{
    for (size_t i = 0; i < BENCHMARK_ITER; i++)
    {
        apbar_sub_portable(apbar_out, apbar_in1, apbar_in2, prec);
        apbar_sub_portable(apbar_out, apbar_in1, apbar_in2, prec);
        apbar_sub_portable(apbar_out, apbar_in1, apbar_in2, prec);
        apbar_sub_portable(apbar_out, apbar_in1, apbar_in2, prec);
    }
}

void barith_sub_intrinsics(unsigned int prec)
{
    for (size_t i = 0; i < BENCHMARK_ITER; i++)
    {
        apbar_sub(apbar_out, apbar_in1, apbar_in2, prec);
        apbar_sub(apbar_out, apbar_in1, apbar_in2, prec);
        apbar_sub(apbar_out, apbar_in1, apbar_in2, prec);
        apbar_sub(apbar_out, apbar_in1, apbar_in2, prec);
    }
}

void barith_sub_merged(unsigned int prec)
{
    for (size_t i = 0; i < BENCHMARK_ITER; i++)
    {
        apbar_sub_merged(apbar_out, apbar_in1, apbar_in2, prec);
        apbar_sub_merged(apbar_out, apbar_in1, apbar_in2, prec);
        apbar_sub_merged(apbar_out, apbar_in1, apbar_in2, prec);
        apbar_sub_merged(apbar_out, apbar_in1, apbar_in2, prec);
    }
}

void barith_sub_midptrep(unsigned int prec)
{
    for (size_t i = 0; i < BENCHMARK_ITER; i++)
    {
        apbar_sub_scalar(apbar_out, apbar_in1, apbar_in2, prec);
        apbar_sub_scalar(apbar_out, apbar_in1, apbar_in2, prec);
        apbar_sub_scalar(apbar_out, apbar_in1, apbar_in2, prec);
        apbar_sub_scalar(apbar_out, apbar_in1, apbar_in2, prec);
    }
}

void barith_sub_nestedbranch(unsigned int prec)
{
    for (size_t i = 0; i < BENCHMARK_ITER; i++)
    {
        apbar_sub_unroll(apbar_out, apbar_in1, apbar_in2, prec);
        apbar_sub_unroll(apbar_out, apbar_in1, apbar_in2, prec);
        apbar_sub_unroll(apbar_out, apbar_in1, apbar_in2, prec);
        apbar_sub_unroll(apbar_out, apbar_in1, apbar_in2, prec);
    }
}

//Name of apbar add functions are misleading. See comments in apfp_add codes for optimization details.
void barith_sub_norad_noexp(unsigned int prec)
{
    for (size_t i = 0; i < BENCHMARK_ITER; i++)
    {
        apbar_sub_unroll_norad_noexp(apbar_out, apbar_in1, apbar_in2, prec);
        apbar_sub_unroll_norad_noexp(apbar_out, apbar_in1, apbar_in2, prec);
        apbar_sub_unroll_norad_noexp(apbar_out, apbar_in1, apbar_in2, prec);
        apbar_sub_unroll_norad_noexp(apbar_out, apbar_in1, apbar_in2, prec);
    }
}

void barith_sub_shiftr(unsigned int prec)
{
    for (size_t i = 0; i < BENCHMARK_ITER; i++)
    {
        apbar_sub_shiftr(apbar_out, apbar_in1, apbar_in2, prec);
        apbar_sub_shiftr(apbar_out, apbar_in1, apbar_in2, prec);
        apbar_sub_shiftr(apbar_out, apbar_in1, apbar_in2, prec);
        apbar_sub_shiftr(apbar_out, apbar_in1, apbar_in2, prec);
    }
}

void barith_sub_shiftl(unsigned int prec)
{
    for (size_t i = 0; i < BENCHMARK_ITER; i++)
    {
        apbar_sub_shiftl(apbar_out, apbar_in1, apbar_in2, prec);
        apbar_sub_shiftl(apbar_out, apbar_in1, apbar_in2, prec);
        apbar_sub_shiftl(apbar_out, apbar_in1, apbar_in2, prec);
        apbar_sub_shiftl(apbar_out, apbar_in1, apbar_in2, prec);
    }
}

void barith_sub_detect1(unsigned int prec)
{
    for (size_t i = 0; i < BENCHMARK_ITER; i++)
    {
        apbar_sub_detect1(apbar_out, apbar_in1, apbar_in2, prec);
        apbar_sub_detect1(apbar_out, apbar_in1, apbar_in2, prec);
        apbar_sub_detect1(apbar_out, apbar_in1, apbar_in2, prec);
        apbar_sub_detect1(apbar_out, apbar_in1, apbar_in2, prec);
    }
}

void barith_mul_no_rad_exp(unsigned int prec)
{
    for (size_t i = 0; i < BENCHMARK_ITER; i++)
    {
        apbar_mul_no_rad_exp(apbar_out, apbar_in1, apbar_in2, prec);
        apbar_mul_no_rad_exp(apbar_out, apbar_in1, apbar_in2, prec);
        apbar_mul_no_rad_exp(apbar_out, apbar_in1, apbar_in2, prec);
        apbar_mul_no_rad_exp(apbar_out, apbar_in1, apbar_in2, prec);
    }
}

static apbar2_t apbar2_out, apbar2_in1, apbar2_in2;
static apbar2_t apbar2_out1, apbar2_in3, apbar2_in4;
static apbar2_t apbar2_out2, apbar2_in5, apbar2_in6;
static apbar2_t apbar2_out3, apbar2_in7, apbar2_in8;

void barith2_init(unsigned int prec)
{
    apbar2_init(apbar2_out, prec);
    apbar2_init(apbar2_in1, prec);
    apbar2_init(apbar2_in2, prec);

    for (uint i = 0; i < apbar2_in1->midpt_size; ++i) {
        apbar2_set_midpt_limb(apbar2_in1, i, urand());
        apbar2_set_midpt_limb(apbar2_in2, i, urand());
    }

    // I am setting the exponents of the midpoint so that the difference can't
    // be more than prec
    apfp_exp_t exp = random();
    apfp_exp_t diff = (apfp_exp_t) ((((double)random() / RAND_MAX) * 2 * prec) - prec);
    apbar2_set_midpt_exp(apbar2_in1, exp);
    apbar2_set_midpt_exp(apbar2_in2, exp + diff);

    apbar2_set_rad(apbar2_in1, (double)rand() / RAND_MAX);
    apbar2_set_rad(apbar2_in2, (double)rand() / RAND_MAX);
}

void barith2_deinit(unsigned int prec)
{
    apbar2_free(apbar2_in1);
    apbar2_free(apbar2_in2);
    apbar2_free(apbar2_out);
}

void barith2_init4(unsigned int prec)
{
    apbar2_init(apbar2_out, prec);
    apbar2_init(apbar2_in1, prec);
    apbar2_init(apbar2_in2, prec);

    apbar2_init(apbar2_out1, prec);
    apbar2_init(apbar2_in3, prec);
    apbar2_init(apbar2_in4, prec);

    apbar2_init(apbar2_out2, prec);
    apbar2_init(apbar2_in5, prec);
    apbar2_init(apbar2_in6, prec);

    apbar2_init(apbar2_out3, prec);
    apbar2_init(apbar2_in7, prec);
    apbar2_init(apbar2_in8, prec);

    for (uint i = 0; i < apbar2_in1->midpt_size; ++i) {
        apbar2_set_midpt_limb(apbar2_in1, i, urand());
        apbar2_set_midpt_limb(apbar2_in2, i, urand());
        apbar2_set_midpt_limb(apbar2_in3, i, urand());
        apbar2_set_midpt_limb(apbar2_in4, i, urand());
        apbar2_set_midpt_limb(apbar2_in5, i, urand());
        apbar2_set_midpt_limb(apbar2_in6, i, urand());
        apbar2_set_midpt_limb(apbar2_in7, i, urand());
        apbar2_set_midpt_limb(apbar2_in8, i, urand());
    }

    // I am setting the exponents of the midpoint so that the difference can't
    // be more than prec
    apfp_exp_t exp = random();
    apfp_exp_t diff1 = (apfp_exp_t) ((((double)random() / RAND_MAX) * 2 * prec) - prec);
    apfp_exp_t diff2 = (apfp_exp_t) ((((double)random() / RAND_MAX) * 2 * prec) - prec);
    apfp_exp_t diff3 = (apfp_exp_t) ((((double)random() / RAND_MAX) * 2 * prec) - prec);
    apfp_exp_t diff4 = (apfp_exp_t) ((((double)random() / RAND_MAX) * 2 * prec) - prec);
    apfp_exp_t diff5 = (apfp_exp_t) ((((double)random() / RAND_MAX) * 2 * prec) - prec);
    apfp_exp_t diff6 = (apfp_exp_t) ((((double)random() / RAND_MAX) * 2 * prec) - prec);
    apfp_exp_t diff7 = (apfp_exp_t) ((((double)random() / RAND_MAX) * 2 * prec) - prec);
    apbar2_set_midpt_exp(apbar2_in1, exp);
    apbar2_set_midpt_exp(apbar2_in2, exp + diff1);
    apbar2_set_midpt_exp(apbar2_in3, exp + diff2);
    apbar2_set_midpt_exp(apbar2_in4, exp + diff3);
    apbar2_set_midpt_exp(apbar2_in5, exp + diff4);
    apbar2_set_midpt_exp(apbar2_in6, exp + diff5);
    apbar2_set_midpt_exp(apbar2_in7, exp + diff6);
    apbar2_set_midpt_exp(apbar2_in8, exp + diff7);

    apbar2_set_rad(apbar2_in1, (double)rand() / RAND_MAX);
    apbar2_set_rad(apbar2_in2, (double)rand() / RAND_MAX);

    apbar2_set_rad(apbar2_in3, (double)rand() / RAND_MAX);
    apbar2_set_rad(apbar2_in4, (double)rand() / RAND_MAX);

    apbar2_set_rad(apbar2_in5, (double)rand() / RAND_MAX);
    apbar2_set_rad(apbar2_in6, (double)rand() / RAND_MAX);

    apbar2_set_rad(apbar2_in7, (double)rand() / RAND_MAX);
    apbar2_set_rad(apbar2_in8, (double)rand() / RAND_MAX);
}

void barith2_deinit4(unsigned int prec)
{
    apbar2_free(apbar2_in1);
    apbar2_free(apbar2_in2);
    apbar2_free(apbar2_out);

    apbar2_free(apbar2_in3);
    apbar2_free(apbar2_in4);
    apbar2_free(apbar2_out1);

    apbar2_free(apbar2_in5);
    apbar2_free(apbar2_in6);
    apbar2_free(apbar2_out2);

    apbar2_free(apbar2_in7);
    apbar2_free(apbar2_in8);
    apbar2_free(apbar2_out3);
}

void barith2_add(unsigned int prec)
{
    for (size_t i = 0; i < BENCHMARK_ITER; i++)
    {
        apbar2_add(apbar2_out, apbar2_in1, apbar2_in2, prec);
        apbar2_add(apbar2_out, apbar2_in1, apbar2_in2, prec);
        apbar2_add(apbar2_out, apbar2_in1, apbar2_in2, prec);
        apbar2_add(apbar2_out, apbar2_in1, apbar2_in2, prec);
    }
}

void barith2_add_optim1(unsigned int prec)
{
    for (size_t i = 0; i < BENCHMARK_ITER; i++)
    {
        apbar2_add_optim1(apbar2_out, apbar2_in1, apbar2_in2, prec);
        apbar2_add_optim1(apbar2_out, apbar2_in1, apbar2_in2, prec);
        apbar2_add_optim1(apbar2_out, apbar2_in1, apbar2_in2, prec);
        apbar2_add_optim1(apbar2_out, apbar2_in1, apbar2_in2, prec);
    }
}

void barith2_add4_scalar(unsigned int prec)
{
    ITERATE({
        apbar2_add_optim1(apbar2_out, apbar2_in1, apbar2_in2, prec);
        apbar2_add_optim1(apbar2_out1, apbar2_in3, apbar2_in4, prec);
        apbar2_add_optim1(apbar2_out2, apbar2_in5, apbar2_in6, prec);
        apbar2_add_optim1(apbar2_out3, apbar2_in7, apbar2_in8, prec);
    });
}

void barith2_add4_simd(unsigned int prec)
{
    ITERATE({
        apbar2_add4(
                apbar2_out, apbar2_in1, apbar2_in2,
                apbar2_out1, apbar2_in3, apbar2_in4,
                apbar2_out2, apbar2_in5, apbar2_in6,
                apbar2_out3, apbar2_in7, apbar2_in8,
                prec
        );
    });
}

void barith2_sub(unsigned int prec)
{
    for (size_t i = 0; i < BENCHMARK_ITER; i++)
    {
        apbar2_sub(apbar2_out, apbar2_in1, apbar2_in2, prec);
        apbar2_sub(apbar2_out, apbar2_in1, apbar2_in2, prec);
        apbar2_sub(apbar2_out, apbar2_in1, apbar2_in2, prec);
        apbar2_sub(apbar2_out, apbar2_in1, apbar2_in2, prec);
    }
}

void barith2_sub_optim1(unsigned int prec)
{
    for (size_t i = 0; i < BENCHMARK_ITER; i++)
    {
        apbar2_sub_optim1(apbar2_out, apbar2_in1, apbar2_in2, prec);
        apbar2_sub_optim1(apbar2_out, apbar2_in1, apbar2_in2, prec);
        apbar2_sub_optim1(apbar2_out, apbar2_in1, apbar2_in2, prec);
        apbar2_sub_optim1(apbar2_out, apbar2_in1, apbar2_in2, prec);
    }
}

void barith2_mul(unsigned int prec)
{
    for (size_t i = 0; i < BENCHMARK_ITER; i++)
    {
        apbar2_mul(apbar2_out, apbar2_in1, apbar2_in2, prec);
        apbar2_mul(apbar2_out, apbar2_in1, apbar2_in2, prec);
        apbar2_mul(apbar2_out, apbar2_in1, apbar2_in2, prec);
        apbar2_mul(apbar2_out, apbar2_in1, apbar2_in2, prec);
    }
}

apint_t in1, in2, out;
static void int_init(uint prec)
{
    apint_init(in1, prec);
    apint_init(in2, prec);
    apint_init(out, prec);
    // apint_init(out, 2 * prec);

    size_t limbs = prec / APINT_LIMB_BITS;
    for (int i = 0; i < limbs; ++i) {
        apint_setlimb(in1, i, urand());
        apint_setlimb(in2, i, urand());
    }
}

static void int_cleanup(uint prec)
{
    apint_free(in1);
    apint_free(in2);
    apint_free(out);
}

static void int_plus(uint prec)
{
    for (size_t i = 0; i < BENCHMARK_ITER; ++i) {
        apint_plus(out, in1, in2);
    }
}

static void int_plus_portable(uint prec)
{
    for (size_t i = 0; i < BENCHMARK_ITER; ++i) {
        apint_plus_portable(out, in1, in2);
    }
}

static void int_mul(uint prec)
{
    for (size_t i = 0; i < BENCHMARK_ITER; ++i) {
        apint_mul(out, in1, in2);
    }
}

static void int_mul_OPT1(uint prec)
{
    for (size_t i = 0; i < BENCHMARK_ITER; ++i)
    {
        apint_mul_OPT1(out, in1, in2);
    }
}
static void int_mul_unroll(uint prec)
{
    ITERATE(apint_mul_unroll(out, in1, in2));
}

static void int_mul_portable(uint prec)
{
    for (size_t i = 0; i < BENCHMARK_ITER; ++i) {
        apint_mul_portable(out, in1, in2);
    }
}

static void int_mul_karatsuba(uint prec)
{
    for (size_t i = 0; i < BENCHMARK_ITER; ++i)
    {
        apint_mul_karatsuba(out, in1, in2);
    }
}

static void int_mul_karatsuba_extend_basecase(uint prec)
{
    for (size_t i = 0; i < BENCHMARK_ITER; ++i)
    {
        apint_mul_karatsuba_extend_basecase(out, in1, in2);
    }
}

apbar_t ball_in1, ball_in2, ball_out;

static void ball_init(uint prec)
{
    apbar_init(ball_in1, prec);
    apbar_init(ball_in2, prec);
    apbar_init(ball_out, prec);

    size_t limbs = prec / APINT_LIMB_BITS;
    for (int i = 0; i < limbs; ++i) {
        apbar_set_midpt_mant(ball_in1, i, urand());
        apbar_set_midpt_mant(ball_in2, i, urand());
    }

    apbar_set_midpt_exp(ball_in1, random());
    apbar_set_midpt_exp(ball_in2, random());

    apbar_set_rad(ball_in1, urand(), random());
    apbar_set_rad(ball_in2, urand(), random());
}

static void ball_cleanup(uint prec)
{
    apbar_free(ball_in1);
    apbar_free(ball_in2);
    apbar_free(ball_out);
}

static void barith_mul_portable(uint prec) {
    ITERATE({
        apbar_mul_portable(ball_out, ball_in1, ball_in2, prec);
        apbar_mul_portable(ball_out, ball_in1, ball_in2, prec);
        apbar_mul_portable(ball_out, ball_in1, ball_in2, prec);
        apbar_mul_portable(ball_out, ball_in1, ball_in2, prec);
    });
}

static void barith_mul_intrinsic(uint prec) {
    ITERATE({
        apbar_mul(ball_out, ball_in1, ball_in2, prec);
        apbar_mul(ball_out, ball_in1, ball_in2, prec);
        apbar_mul(ball_out, ball_in1, ball_in2, prec);
        apbar_mul(ball_out, ball_in1, ball_in2, prec);
    });
}

static void barith_mul_rad_opt(uint prec) {
    ITERATE({
        apbar_mul_no_rad_exp(ball_out, ball_in1, ball_in2, prec);
        apbar_mul_no_rad_exp(ball_out, ball_in1, ball_in2, prec);
        apbar_mul_no_rad_exp(ball_out, ball_in1, ball_in2, prec);
        apbar_mul_no_rad_exp(ball_out, ball_in1, ball_in2, prec);
    });
}
static void barith_mul_unroll(uint prec) {
    ITERATE({
        apbar_mul_unroll(ball_out, ball_in1, ball_in2, prec);
        apbar_mul_unroll(ball_out, ball_in1, ball_in2, prec);
        apbar_mul_unroll(ball_out, ball_in1, ball_in2, prec);
        apbar_mul_unroll(ball_out, ball_in1, ball_in2, prec);
    });
}

static void int_mul_karatsuba_opt1(uint prec)
{
    for (size_t i = 0; i < BENCHMARK_ITER; ++i)
    {
        apint_mul_karatsuba_OPT1(out, in1, in2);
    }
}

BENCHMARK_BEGIN_SUITE()
BENCHMARK_BEGIN_TABLE(def)
    BENCHMARK_FUNCTION(arblib_add, arblib_init, arblib_deinit, 4.0, 8, 17)
    BENCHMARK_FUNCTION(barith_add, barith_init, barith_deinit, 4.0, 8, 17)
    BENCHMARK_FUNCTION(barith2_add, barith2_init, barith2_deinit, 4.0, 8, 17)
BENCHMARK_END_TABLE(def)

BENCHMARK_BEGIN_TABLE(ball_add_micro)
    BENCHMARK_FUNCTION(barith_add_portable, barith_init, barith_deinit, 4.0, 8, 17)
    BENCHMARK_FUNCTION(barith_add, barith_init, barith_deinit, 4.0, 8, 17)
    BENCHMARK_FUNCTION(barith_add_shiftr, barith_init, barith_deinit, 4.0, 8, 17)
    BENCHMARK_FUNCTION(barith_add_shiftl, barith_init, barith_deinit, 4.0, 8, 17)
    BENCHMARK_FUNCTION(barith_add_detect1, barith_init, barith_deinit, 4.0, 8, 17)
BENCHMARK_END_TABLE(ball_add_micro)

BENCHMARK_BEGIN_TABLE(ball_add_old)
    BENCHMARK_FUNCTION(barith_add_portable, barith_init, barith_deinit, 4.0, 8, 17)
    BENCHMARK_FUNCTION(barith_add_intrinsics, barith_init, barith_deinit, 4.0, 8, 17)
    BENCHMARK_FUNCTION(barith_add_merged, barith_init, barith_deinit, 4.0, 8, 17)
    BENCHMARK_FUNCTION(barith_add_midptrep, barith_init, barith_deinit, 4.0, 8, 17)
    BENCHMARK_FUNCTION(barith_add_nestedbranch, barith_init, barith_deinit, 4.0, 8, 17)
    BENCHMARK_FUNCTION(barith_add_norad_noexp, barith_init, barith_deinit, 4.0, 8, 17)
BENCHMARK_END_TABLE(ball_add_old)

BENCHMARK_BEGIN_TABLE(ball_add_all)
    BENCHMARK_FUNCTION(barith_add_portable, barith_init, barith_deinit, 4.0, 8, 20)
    BENCHMARK_FUNCTION(barith_add_intrinsics, barith_init, barith_deinit, 4.0, 8, 20)
    BENCHMARK_FUNCTION(barith_add_shiftr, barith_init, barith_deinit, 4.0, 8, 20)
    BENCHMARK_FUNCTION(barith_add_shiftl, barith_init, barith_deinit, 4.0, 8, 20)
    BENCHMARK_FUNCTION(barith_add_detect1, barith_init, barith_deinit, 4.0, 8, 20)
    BENCHMARK_FUNCTION(barith_add_merged, barith_init, barith_deinit, 4.0, 8, 20)
    BENCHMARK_FUNCTION(barith_add_midptrep, barith_init, barith_deinit, 4.0, 8, 20)
    BENCHMARK_FUNCTION(barith_add_nestedbranch, barith_init, barith_deinit, 4.0, 8, 20)
    BENCHMARK_FUNCTION(barith_add_norad_noexp, barith_init, barith_deinit, 4.0, 8, 20)
    BENCHMARK_FUNCTION(barith2_add, barith2_init, barith2_deinit, 4.0, 8, 20)
    BENCHMARK_FUNCTION(barith2_add_optim1, barith2_init, barith2_deinit, 4.0, 8, 20)
BENCHMARK_END_TABLE(ball_add_all)

BENCHMARK_BEGIN_TABLE(ball_sub_micro)
    BENCHMARK_FUNCTION(barith_sub_intrinsics, barith_init, barith_deinit, 4.0, 8, 17)
    BENCHMARK_FUNCTION(barith_sub_shiftr, barith_init, barith_deinit, 4.0, 8, 17)
    BENCHMARK_FUNCTION(barith_sub_shiftl, barith_init, barith_deinit, 4.0, 8, 17)
    BENCHMARK_FUNCTION(barith_sub_detect1, barith_init, barith_deinit, 4.0, 8, 17)
BENCHMARK_END_TABLE(ball_sub_micro)

BENCHMARK_BEGIN_TABLE(ball_sub_old)
        BENCHMARK_FUNCTION(barith_sub_intrinsics, barith_init, barith_deinit, 4.0, 8, 17)
        BENCHMARK_FUNCTION(barith_sub_merged, barith_init, barith_deinit, 4.0, 8, 17)
        BENCHMARK_FUNCTION(barith_sub_midptrep, barith_init, barith_deinit, 4.0, 8, 17)
        BENCHMARK_FUNCTION(barith_sub_nestedbranch, barith_init, barith_deinit, 4.0, 8, 17)
        BENCHMARK_FUNCTION(barith_sub_norad_noexp, barith_init, barith_deinit, 4.0, 8, 17)
BENCHMARK_END_TABLE(ball_sub_old)

BENCHMARK_BEGIN_TABLE(ball_sub_all)
    BENCHMARK_FUNCTION(barith_sub_portable, barith_init, barith_deinit, 4.0, 8, 20)
    BENCHMARK_FUNCTION(barith_sub_intrinsics, barith_init, barith_deinit, 4.0, 8, 20)
    BENCHMARK_FUNCTION(barith_sub_shiftr, barith_init, barith_deinit, 4.0, 8, 20)
    BENCHMARK_FUNCTION(barith_sub_shiftl, barith_init, barith_deinit, 4.0, 8, 20)
    BENCHMARK_FUNCTION(barith_sub_detect1, barith_init, barith_deinit, 4.0, 8, 20)
    BENCHMARK_FUNCTION(barith_sub_merged, barith_init, barith_deinit, 4.0, 8, 20)
    BENCHMARK_FUNCTION(barith_sub_midptrep, barith_init, barith_deinit, 4.0, 8, 20)
    BENCHMARK_FUNCTION(barith_sub_nestedbranch, barith_init, barith_deinit, 4.0, 8, 20)
    BENCHMARK_FUNCTION(barith_sub_norad_noexp, barith_init, barith_deinit, 4.0, 8, 20)
    BENCHMARK_FUNCTION(barith2_sub, barith2_init, barith2_deinit, 4.0, 8, 20)
    BENCHMARK_FUNCTION(barith2_sub_optim1, barith2_init, barith2_deinit, 4.0, 8, 20)
BENCHMARK_END_TABLE(ball_sub_all)

BENCHMARK_BEGIN_TABLE(ball_add_new_old)
    BENCHMARK_FUNCTION(barith_add_nestedbranch, barith_init, barith_deinit, 4.0, 8, 17)
    BENCHMARK_FUNCTION(barith2_add, barith2_init, barith2_deinit, 4.0, 8, 17)
    BENCHMARK_FUNCTION(barith2_add_optim1, barith2_init, barith2_deinit, 4.0, 8, 17)
BENCHMARK_END_TABLE(ball_add_new_old)

BENCHMARK_BEGIN_TABLE(ball_sub_new_old)
    BENCHMARK_FUNCTION(barith_sub_nestedbranch, barith_init, barith_deinit, 4.0, 8, 17)
    BENCHMARK_FUNCTION(barith2_sub, barith2_init, barith2_deinit, 4.0, 8, 17)
    BENCHMARK_FUNCTION(barith2_sub_optim1, barith2_init, barith2_deinit, 4.0, 8, 17)
BENCHMARK_END_TABLE(ball_sub_new_old)

BENCHMARK_BEGIN_TABLE(ball_mul_all)
    BENCHMARK_FUNCTION(barith_mul_portable, ball_init, ball_cleanup, 4.0, 8, 20)
    BENCHMARK_FUNCTION(barith_mul_intrinsic, ball_init, ball_cleanup, 4.0, 8, 20)
    BENCHMARK_FUNCTION(barith_mul_rad_opt, ball_init, ball_cleanup, 4.0, 8, 20)
    BENCHMARK_FUNCTION(barith_mul_unroll, ball_init, ball_cleanup, 4.0, 8, 20)
    BENCHMARK_FUNCTION(barith2_mul, barith2_init, barith2_deinit, 4.0, 8, 20)
BENCHMARK_END_TABLE(ball_mul_all)

BENCHMARK_BEGIN_TABLE(ball_mult_new_old)
    BENCHMARK_FUNCTION(barith_mul_no_rad_exp, barith_init, barith_deinit, 4.0, 8, 17)
    BENCHMARK_FUNCTION(barith2_mul, barith2_init, barith2_deinit, 4.0, 8, 17)
BENCHMARK_END_TABLE(ball_mult_new_old)

BENCHMARK_BEGIN_TABLE(compare_add_arblib)
    BENCHMARK_FUNCTION(barith_add_portable, barith_init, barith_deinit, 4.0, 8, 20)
    BENCHMARK_FUNCTION(arblib_add, arblib_init, arblib_deinit, 4.0, 8, 20)
    BENCHMARK_FUNCTION(barith_add_nestedbranch, barith_init, barith_deinit, 4.0, 8, 20)
    BENCHMARK_FUNCTION(barith2_add_optim1, barith2_init, barith2_deinit, 4.0, 8, 20)
BENCHMARK_END_TABLE(compare_add_arblib)

BENCHMARK_BEGIN_TABLE(compare_subtract_arblib)
    BENCHMARK_FUNCTION(barith_sub_portable, barith_init, barith_deinit, 4.0, 8, 20)
    BENCHMARK_FUNCTION(arblib_sub, arblib_init, arblib_deinit, 4.0, 8, 20)
    BENCHMARK_FUNCTION(barith_sub_nestedbranch, barith_init, barith_deinit, 4.0, 8, 20)
    BENCHMARK_FUNCTION(barith2_sub_optim1, barith2_init, barith2_deinit, 4.0, 8, 20)
BENCHMARK_END_TABLE(compare_subtract_arblib)

BENCHMARK_BEGIN_TABLE(compare_mul_arblib)
    BENCHMARK_FUNCTION(arblib_mul, arblib_init, arblib_deinit, 4.0, 8, 17)
    BENCHMARK_FUNCTION(barith_mul_portable, ball_init, ball_cleanup, 4.0, 8, 17)
    BENCHMARK_FUNCTION(barith_mul_no_rad_exp, barith_init, barith_deinit, 4.0, 8, 17)
    BENCHMARK_FUNCTION(barith2_mul, barith2_init, barith2_deinit, 4.0, 8, 17)
BENCHMARK_END_TABLE(compare_mul_arblib)

BENCHMARK_BEGIN_TABLE(int_plus)
    BENCHMARK_FUNCTION(int_plus, int_init, int_cleanup, 1.0, 8, 17)
    BENCHMARK_FUNCTION(int_plus_portable, int_init, int_cleanup, 1.0, 8, 17)
BENCHMARK_END_TABLE(int_plus)

BENCHMARK_BEGIN_TABLE(karatsuba)
    BENCHMARK_FUNCTION(int_mul_unroll, int_init, int_cleanup, 1.0, 8, 17)
    BENCHMARK_FUNCTION(int_mul_karatsuba, int_init, int_cleanup, 1.0, 8, 17)
    BENCHMARK_FUNCTION(int_mul_karatsuba_extend_basecase, int_init, int_cleanup, 1.0, 8, 17)
    BENCHMARK_FUNCTION(int_mul_karatsuba_opt1, int_init, int_cleanup, 1.0, 8, 17)
BENCHMARK_END_TABLE(karatsuba)

BENCHMARK_BEGIN_TABLE(ball_add_simd)
    BENCHMARK_FUNCTION(barith2_add4_scalar, barith2_init4, barith2_deinit4, 1.0, 8, 20)
    BENCHMARK_FUNCTION(barith2_add4_simd, barith2_init4, barith2_deinit4, 1.0, 8, 20)
BENCHMARK_END_TABLE(ball_add_simd)

BENCHMARK_BEGIN_TABLE(compiler)
    BENCHMARK_FUNCTION(barith2_add_optim1, barith2_init, barith2_deinit, 4.0, 8, 20)
    BENCHMARK_FUNCTION(barith2_sub_optim1, barith2_init, barith2_deinit, 4.0, 8, 20)
    BENCHMARK_FUNCTION(barith2_mul, barith2_init, barith2_deinit, 4.0, 8, 20)
BENCHMARK_END_TABLE(compiler)

//BENCHMARK_BEGIN_TABLE(apbar_mul)
//    BENCHMARK_FUNCTION(ball_mul_portable, ball_init, ball_cleanup, 1.0, 8, 18)
//    BENCHMARK_FUNCTION(ball_mul, ball_init, ball_cleanup, 1.0, 8, 18)
//    BENCHMARK_FUNCTION(ball_mul_no_exp, ball_init, ball_cleanup, 1.0, 8, 18)
//    BENCHMARK_FUNCTION(ball_mul_unroll, ball_init, ball_cleanup, 1.0, 8, 18)
//BENCHMARK_END_TABLE(apbar_mul)
//
//BENCHMARK_BEGIN_TABLE(apbar_mul_small)
//    BENCHMARK_FUNCTION(ball_mul_portable, ball_init, ball_cleanup, 1.0, 8, 12)
//    BENCHMARK_FUNCTION(ball_mul, ball_init, ball_cleanup, 1.0, 8, 12)
//    BENCHMARK_FUNCTION(ball_mul_no_exp, ball_init, ball_cleanup, 1.0, 8, 12)
//    BENCHMARK_FUNCTION(ball_mul_unroll, ball_init, ball_cleanup, 1.0, 8, 12)
//BENCHMARK_END_TABLE(apbar_mul_small)
//
//BENCHMARK_BEGIN_TABLE(apbar_mul_large)
//    BENCHMARK_FUNCTION(ball_mul_portable, ball_init, ball_cleanup, 1.0, 12, 18)
//    BENCHMARK_FUNCTION(ball_mul, ball_init, ball_cleanup, 1.0, 12, 18)
//    BENCHMARK_FUNCTION(ball_mul_no_exp, ball_init, ball_cleanup, 1.0, 12, 18)
//    BENCHMARK_FUNCTION(ball_mul_unroll, ball_init, ball_cleanup, 1.0, 12, 18)
//BENCHMARK_END_TABLE(apbar_mul_large)

BENCHMARK_END_SUITE()

int main(int argc, char const *argv[])
{
    const char *suite_name;
    if (argc < 2) {
        suite_name = "def";
    }
    else {
        suite_name = argv[1];
    }
    BENCHMARK_FOREACH(current, suite_name)
    {
        for (unsigned int p = current->prec_start; p < current->prec_stop; p++)
        {
            unsigned int precision = 1u << p;

            current->init(precision);
            double cycles = bench(current->function, precision) / BENCHMARK_ITER / current->divisor;
            current->deinit(precision);

            printf("%-14s %6d bits : %.2f cyc\n", current->name, precision, cycles);
        }
    }

    return 0;
}
