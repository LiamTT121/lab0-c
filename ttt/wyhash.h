/* adapted to this project by D. Lemire,
 * from https://github.com/wangyi-fudan/wyhash/blob/master/wyhash.h
 *      https://github.com/lemire/testingRNG/blob/master/source/wyhash.h
 * This uses mum hashing.
 */

#include <stdint.h>

/* call wyhash64_seed before calling wyhash64 */
void wyhash64_seed(uint64_t seed);

/* returns random number, modifies wyhash64_x */
uint64_t wyhash64(void);

/* returns the 32 least significant bits of a call to wyhash64
 * this is a simple function call followed by a cast
 */
uint32_t wyhash64_cast32(void);

/* Reference:
 * https://github.com/colmmacc/s2n/blob/ \
 * 7ad9240c8b9ade0cc3a403a732ba9f1289934abd/utils/s2n_random.c#L187
 */
uint64_t lemire_rand(uint64_t bound);
