#ifndef FIX_POINT_H
#define FIX_POINT_H

#include <stdint.h>

#define FRAC_DIGIT 15
#define FIXED_1 ((uint32_t) 1 << FRAC_DIGIT)
#define POINT_FIVE (FIXED_1 >> 1)
#define FIX_POINT_MAX (~((uint32_t) 0))

#define INT_TO_FIX_POINT(x) ((uint32_t) (x) << FRAC_DIGIT)

uint32_t fix_div(uint32_t nume, uint32_t deno);
uint32_t fix_mul(uint32_t x, uint32_t y);
uint32_t fix_sqrt(uint32_t x);

#endif
