#include <arb.h>
#include <stdio.h>
#include <flint/flint.h>

#include "apbar.h"
#include "tsc_x86.h"
#include "benchmark.h"

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
    arb_clear(arb_out); arb_clear(arb_in1); arb_clear(arb_in2);
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

static apbar_t apbar_out, apbar_in1, apbar_in2;

void barith_init(unsigned int prec)
{
    apbar_init(apbar_out, prec);
    apbar_init(apbar_in1, prec);
    apbar_init(apbar_in2, prec);

    // TODO: Use arbitrary precision random number.
    apbar_set_d(apbar_in1, (double)rand() / RAND_MAX);
    apbar_set_d(apbar_in2, (double)rand() / RAND_MAX);
}

void barith_deinit(unsigned int prec)
{
    apbar_free(apbar_in1);
    apbar_free(apbar_in2);
    apbar_free(apbar_out);
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

BENCHMARK_BEGIN_SUITE()
BENCHMARK_BEGIN_TABLE(def)
    BENCHMARK_FUNCTION(arblib_add, arblib_init, arblib_deinit, 4.0, 8, 17)
    BENCHMARK_FUNCTION(barith_add, barith_init, barith_deinit, 4.0, 8, 17)
BENCHMARK_END_TABLE(def)

// To-do: Benchmark the other apbar methods.

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
