/* adapted to this project by D. Lemire,
 * from https://github.com/wangyi-fudan/wyhash/blob/master/wyhash.h
 *      https://github.com/lemire/testingRNG/blob/master/source/wyhash.h
 * This uses mum hashing.
 */
#include <stdint.h>

#include "wyhash.h"

/* state for wyhash64
 * The state can be seeded with any value.
 */
static uint64_t wyhash64_x;

/* call wyhash64_seed before calling wyhash64 */
void wyhash64_seed(uint64_t seed)
{
    wyhash64_x = seed;
}

static uint64_t wyhash64_stateless(uint64_t *seed)
{
    *seed += UINT64_C(0x60bee2bee120fc15);
    __uint128_t tmp;
    tmp = (__uint128_t) *seed * UINT64_C(0xa3b195354a39b70d);
    uint64_t m1 = (tmp >> 64) ^ tmp;
    tmp = (__uint128_t) m1 * UINT64_C(0x1b03738712fad5c9);
    uint64_t m2 = (tmp >> 64) ^ tmp;
    return m2;
}

/* returns random number, modifies wyhash64_x */
uint64_t wyhash64(void)
{
    return wyhash64_stateless(&wyhash64_x);
}

/* returns the 32 least significant bits of a call to wyhash64
 * this is a simple function call followed by a cast
 */
uint32_t wyhash64_cast32(void)
{
    return (uint32_t) wyhash64();
}

uint64_t lemire_rand(uint64_t bound)
{
    uint64_t x = wyhash64();
    __uint128_t m = (__uint128_t) x * (__uint128_t) bound;
    uint64_t l = (uint64_t) m;
    if (l < bound) {
        uint64_t floor = -bound % bound;
        while (l < floor) {
            x = wyhash64();
            m = (__uint128_t) x * (__uint128_t) bound;
            l = (uint64_t) m;
        }
    }
    return m >> 64;
}
