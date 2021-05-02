#include "apbar.h"

void apbar_init(apbar_t x, apint_size_t p)
{    apfp_init(x->midpt, p);
}

void apbar_add(apbar_ptr c, apbar_srcptr a, apbar_srcptr b, apint_size_t p)
{
    apbar_init(c, p);
    apfp_add(c->midpt, a->midpt, b->midpt);
    c->rad = a->rad + b->rad;
}