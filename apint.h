#ifndef APINT_H
#define APINT_H

#include <stdint.h>

typedef struct
{
    uint64_t *values;
    size_t length;
} apint_t;

void apint_add(apint_t *dst, apint_t *v1, apint_t *v2);

#endif /* !APINT_H */
