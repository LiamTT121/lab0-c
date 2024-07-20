#ifndef FIX_POINT_H
#define FIX_POINT_H

#include <stdint.h>

#define FRAC_DIGIT 15

#define INT_TO_FIX_POINT(x) ((uint32_t) (x) << FRAC_DIGIT)

inline uint32_t fix_div(uint32_t deno, uint32_t nume)
{
    deno /= nume;
    return deno << FRAC_DIGIT;
}

inline uint32_t fix_mul(uint32_t x, uint32_t y)
{
    x *= y;
    return x >> FRAC_DIGIT;
}

inline uint32_t fix_log2(uint32_t x)
{
    return 31 - __builtin_clz((x >> FRAC_DIGIT) | 1);
}

inline uint32_t fix_sqrt(uint32_t x)
{
    uint32_t y, d, c = 0;

    if (x <= 1)
        return x;

    d = 1UL << (fix_log2(x) & ~1UL);
    while (d != 0) {
        y = c + d;
        c >>= 1;

        if (x >= y) {
            x -= y;
            c += d;
        }
        d >>= 2;
    }
    return c;
}

#endif
