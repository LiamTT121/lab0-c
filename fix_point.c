#include <stdint.h>

#include "fix_point.h"

uint32_t fix_div(uint32_t nume, uint32_t deno)
{
    return nume / (deno >> FRAC_DIGIT);
}

uint32_t fix_mul(uint32_t x, uint32_t y)
{
    x *= y;
    return x >> FRAC_DIGIT;
}

static inline uint64_t int_log2(uint64_t x)
{
    return 63 - __builtin_clzl(x | 1);
}

static inline uint32_t int64_sqrt(uint64_t x)
{
    uint64_t y, d, c = 0;

    if (x <= 1)
        return x;

    d = 1UL << (int_log2(x) & ~1UL);
    while (d != 0) {
        y = c + d;
        c >>= 1;

        if (x >= y) {
            x -= y;
            c += d;
        }
        d >>= 2;
    }
    return (uint32_t) c;
}

uint32_t fix_sqrt(uint32_t x)
{
    return int64_sqrt((uint64_t) x << FRAC_DIGIT);
}
