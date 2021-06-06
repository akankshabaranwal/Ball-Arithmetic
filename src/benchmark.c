#include <arb.h>
#include <stdio.h>
#include <flint/flint.h>

#include "apbar.h"
#include "apbar2.h"
#include "tsc_x86.h"
#include "benchmark.h"

#define ITERATE(CODE) for (size_t i = 0; i < BENCHMARK_ITER; i++) (CODE);

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
static apbar_t apbar_out, apbar_in1, apbar_in2;

void barith_init(unsigned int prec)
{
    apbar_init(apbar_out, prec);
    apbar_init(apbar_in1, prec);
    apbar_init(apbar_in2, prec);

    // TODO: Use arbitrary precision random number.
    apbar_set_d(apbar_in1, (double)rand() / RAND_MAX);
    apbar_in1->midpt->mant->sign =1;
    apbar_set_d(apbar_in2, (double)rand() / RAND_MAX);
    apbar_in2->midpt->mant->sign =1;
}

void barith_deinit(unsigned int prec)
{
    apbar_free(apbar_in1);
    apbar_free(apbar_in2);
    apbar_free(apbar_out);
}

void barith_add_base(unsigned int prec)
{
    for (size_t i = 0; i < BENCHMARK_ITER; i++)
    {
        apbar_add_base(apbar_out, apbar_in1, apbar_in2, prec);
        apbar_add_base(apbar_out, apbar_in1, apbar_in2, prec);
        apbar_add_base(apbar_out, apbar_in1, apbar_in2, prec);
        apbar_add_base(apbar_out, apbar_in1, apbar_in2, prec);
    }
}

void barith_add_optim1(unsigned int prec)
{
    for (size_t i = 0; i < BENCHMARK_ITER; i++)
    {
        apbar_add_optim1(apbar_out, apbar_in1, apbar_in2, prec);
        apbar_add_optim1(apbar_out, apbar_in1, apbar_in2, prec);
        apbar_add_optim1(apbar_out, apbar_in1, apbar_in2, prec);
        apbar_add_optim1(apbar_out, apbar_in1, apbar_in2, prec);
    }
}

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


void barith_sub(unsigned int prec)
{
    for (size_t i = 0; i < BENCHMARK_ITER; i++)
    {
        apbar_sub(apbar_out, apbar_in1, apbar_in2, prec);
        apbar_sub(apbar_out, apbar_in1, apbar_in2, prec);
        apbar_sub(apbar_out, apbar_in1, apbar_in2, prec);
        apbar_sub(apbar_out, apbar_in1, apbar_in2, prec);
    }
}



static apbar2_t apbar2_out, apbar2_in1, apbar2_in2;

void barith2_init(unsigned int prec)
{
    apbar2_init(apbar2_out, prec);
    apbar2_init(apbar2_in1, prec);
    apbar2_init(apbar2_in2, prec);

    // TODO: Use arbitrary precision random number.
    apbar2_set_d(apbar2_in1, (double)rand() / RAND_MAX);
    apbar2_set_d(apbar2_in2, (double)rand() / RAND_MAX);
}

void barith2_deinit(unsigned int prec)
{
    apbar2_free(apbar2_in1);
    apbar2_free(apbar2_in2);
    apbar2_free(apbar2_out);
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

static void ball_mull_vanilla(uint prec) {
    ITERATE(apbar_mul(ball_out, ball_in1, ball_in2, prec));
}

static void ball_mull_no_exp(uint prec) {
    ITERATE(apbar_mul_no_rad_exp(ball_out, ball_in1, ball_in2, prec));
}
static void ball_mull_unroll(uint prec) {
    ITERATE(apbar_mul_unroll(ball_out, ball_in1, ball_in2, prec));
}

BENCHMARK_BEGIN_SUITE()
BENCHMARK_BEGIN_TABLE(def)
    BENCHMARK_FUNCTION(arblib_add, arblib_init, arblib_deinit, 4.0, 8, 17)
    BENCHMARK_FUNCTION(barith_add, barith_init, barith_deinit, 4.0, 8, 17)
    BENCHMARK_FUNCTION(barith2_add, barith2_init, barith2_deinit, 4.0, 8, 17)
BENCHMARK_END_TABLE(def)

BENCHMARK_BEGIN_TABLE(ball_add)
                BENCHMARK_FUNCTION(arblib_add, arblib_init, arblib_deinit, 4.0, 8, 17)
                BENCHMARK_FUNCTION(barith_add_base, barith_init, barith_deinit, 4.0, 8, 17)
                BENCHMARK_FUNCTION(barith_add_optim1, barith_init, barith_deinit, 4.0, 8, 17)
                BENCHMARK_FUNCTION(barith_add, barith_init, barith_deinit, 4.0, 8, 17)
                BENCHMARK_FUNCTION(barith2_add, barith2_init, barith2_deinit, 4.0, 8, 17)
BENCHMARK_END_TABLE(ball_add)

BENCHMARK_BEGIN_TABLE(ball_sub)
                BENCHMARK_FUNCTION(arblib_sub, arblib_init, arblib_deinit, 4.0, 8, 17)
                //BENCHMARK_FUNCTION(barith_add_base, barith_init, barith_deinit, 4.0, 8, 17)
                //BENCHMARK_FUNCTION(barith_add_optim1, barith_init, barith_deinit, 4.0, 8, 17)
                BENCHMARK_FUNCTION(barith_sub, barith_init, barith_deinit, 4.0, 8, 17)
                BENCHMARK_FUNCTION(barith2_sub, barith2_init, barith2_deinit, 4.0, 8, 17)
BENCHMARK_END_TABLE(ball_sub)

BENCHMARK_BEGIN_TABLE(int_plus)
    BENCHMARK_FUNCTION(int_plus, int_init, int_cleanup, 1.0, 8, 17)
    BENCHMARK_FUNCTION(int_plus_portable, int_init, int_cleanup, 1.0, 8, 17)
BENCHMARK_END_TABLE(int_plus)

BENCHMARK_BEGIN_TABLE(int_mul)
    BENCHMARK_FUNCTION(int_mul, int_init, int_cleanup, 1.0, 8, 17)
    BENCHMARK_FUNCTION(int_mul_portable, int_init, int_cleanup, 1.0, 8, 17)
    BENCHMARK_FUNCTION(int_mul_karatsuba, int_init, int_cleanup, 1.0, 8, 17)
    BENCHMARK_FUNCTION(int_mul_karatsuba_extend_basecase, int_init, int_cleanup, 1.0, 8, 17)
BENCHMARK_END_TABLE(int_mul)

BENCHMARK_BEGIN_TABLE(apbar_mul)
    BENCHMARK_FUNCTION(ball_mull_vanilla, ball_init, ball_cleanup, 1.0, 8, 17)
    BENCHMARK_FUNCTION(ball_mull_no_exp, ball_init, ball_cleanup, 1.0, 8, 17)
    BENCHMARK_FUNCTION(ball_mull_unroll, ball_init, ball_cleanup, 1.0, 8, 17)
BENCHMARK_END_TABLE(apbar_mul)
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

            printf("%-14s %5d bits : %.2f cyc\n", current->name, precision, cycles);
        }
    }

    return 0;
}
