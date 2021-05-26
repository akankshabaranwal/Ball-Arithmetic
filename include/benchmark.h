#ifndef BALL_ARITHMETIC_BENCHMARK_H
#define BALL_ARITHMETIC_BENCHMARK_H

static const int BENCHMARK_ITER = 50000;
static const int BENCHMARK_WARMUPS = 30;

typedef void (*benchmark_fun_t)(unsigned int precision);

typedef struct benchmark_struct_t
{
    benchmark_fun_t function;
    benchmark_fun_t init;
    benchmark_fun_t deinit;
    char *name;
    double divisor;
    unsigned int prec_start;
    unsigned int prec_stop;
} benchmark_t;

#define BENCHMARK_BEGIN_TABLE() benchmark_t _TABLE[] = {
#define BENCHMARK_END_TABLE()   { 0 }};
#define BENCHMARK_FUNCTION(_function, _init, _deinit, _divisor, _prec_start, _prec_stop) \
    {                               \
        .name = #_function,         \
        .function = _function,      \
        .init = _init,              \
        .deinit = _deinit,          \
        .divisor = _divisor,        \
        .prec_start = _prec_start,  \
        .prec_stop = _prec_stop     \
    },

#define BENCHMARK_FOREACH(_iterator) \
    for (benchmark_t *_iterator = _TABLE; _iterator->function != NULL; _iterator++)

#endif // !BALL_ARITHMETIC_BENCHMARK_H
