#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "hash_murmur3_nose.h"

#define BENCHMARK

#ifdef BENCHMARK
#include <fcntl.h>
#include <time.h>
#include <unistd.h>

#define LOOP_SIZE 10000000

#define START_BENCH(start) start = (float)clock() / CLOCKS_PER_SEC

#define STOP_BENCH(start, stop, diff, result)                                  \
	stop   = (float)clock() / CLOCKS_PER_SEC;                                  \
	diff   = stop - start;                                                     \
	result = diff;

#else
#define LOOP_SIZE 0
#define START_BENCH(start)
#define STOP_BENCH(start, stop, diff, result)
#define BENCH(expression, loop, result)
#endif /* BENCHMARK */

//------------------------------------------------------------------- MACROS ---
#define ARRAY_LENGTH(_array) (sizeof(_array) / sizeof((_array)[0]))

//------------------------------------------------------------- DECLARATIONS ---

//----------------------------------------------------------------- Function ---
/**
 * Compute modulus division by 2^n
 *   -> Remainder
 */

//----------------------------------------------------------------- Function ---
/**
 * Returns a multiplicative style hash for given key
 *
 * see : Linear congruential generator
 */
static inline size_t hash_multiplicative(const char *key, const size_t seed)
    __attribute__((const, always_inline));

static inline size_t hash_multiplicative(const char *key, const size_t seed)
{
	size_t hash = seed;

	while (*key) {
		hash = seed * hash + *key++;
	}

	return hash;
}

static inline size_t hash_fimur_reduce(const char *key,
                                       const size_t seed,
                                       const unsigned int n)
   __attribute__((const, always_inline));

static inline size_t hash_fimur_reduce(const char *key,
                                       const size_t seed,
                                       const unsigned int n)
{
	size_t hash = seed;

	while (*key) {
		hash *= 11400714819323198485llu;
		hash *= *(key++) << 15;
		hash = (hash << 7) | (hash >> (32 - 7));
	}

	return hash >> (64 - n);
}

//----------------------------------------------------------------- Function ---
/**
 * Compute modulus division by 2^n - 1 for given integer dividend
 *   -> Remainder
 */
static inline size_t mod_2n1(uint_fast64_t dividend, const unsigned int n)
{
	const uint_fast64_t divisor = (1u << n) - 1;
	uint64_t remainder;

	for (remainder = dividend; dividend > divisor; dividend = remainder) {
		for (remainder = 0; dividend; dividend >>= n) {
			remainder += dividend & divisor;
		}
	}
	return remainder == divisor ? 0 : remainder;
}

//----------------------------------------------------------------- Function ---
/**
 * Compute modulus division by 2^n - 1 for given Hash128
 *   -> Remainder
 */
static inline size_t mod_hash128(const Hash128 hash, const unsigned int n)
{
	const uint32_t h1 = (hash.hi & 0xFFFFFFFF00000000ul) >> 32;
	const uint32_t h2 = (hash.hi & 0x00000000FFFFFFFFul);
	const uint32_t h3 = (hash.lo & 0xFFFFFFFF00000000ul) >> 32;
	const uint32_t h4 = (hash.lo & 0x00000000FFFFFFFFul);

	return mod_2n1(
	    (mod_2n1((mod_2n1((mod_2n1(h1, n) << 32) + h2, n) << 32) + h3, n)
	     << 32) +
	        h4,
	    n);
}
//--------------------------------------------------------------------- MAIN ---
int main(void)
{
//------------------------------------------------------------ setup
#define KEYPOOL_SIZE 4096
	char random_keys[KEYPOOL_SIZE] = {'\0'};
	int r;

	for (size_t i = 0; i < KEYPOOL_SIZE; i++) {
		r = rand() % 32;
		// Replace with a-z for interval [0,25]
		// Leave '\0' for interval [26,32]
		if (r < 26) {
			random_keys[i] = (char)(r + 0x61);
			putchar(random_keys[i]);
		}
	}

	/**
	 * Get a key ( including '\0' !! ) by pointing somewhere in random_keys
	 */
	// for (size_t i = 0; i < 10000; i++)
	// {
	// 	printf("\n%s", &random_keys[rand() % 4096]);
	// }

	/**
	 * Get all keys ( including '\0' !! )
	 */
	// for (size_t i = 0; i < KEYPOOL_SIZE; i++)
	// {
	// 	printf("\n%s", &random_keys[i]);
	// }

	puts("\nhash tests :\n");
	size_t hash;
	uint_fast32_t hash32;
	Hash128 hash128;

	uint_fast16_t seed = 41;

	size_t test_count = 100000;

	float start, stop, diff, bench_time;

	//---------------------------------------------------------- bench A
	START_BENCH(start);
	for (size_t i = 0; i < test_count; i++) {
		for (size_t i = 0; i < KEYPOOL_SIZE; i++) {
			hash = hash_multiplicative(&random_keys[i], seed);
			// Shove strlen as seed to make comparison fair
			// hash =
			//     hash_multiplicative(&random_keys[i],
			//     strlen(&random_keys[i]));
			// printf("%016lx : %s\n", hash, &random_keys[i]);
		}
	}
	STOP_BENCH(start, stop, diff, bench_time);
	printf("bench hash_multiplicative   \t: %f \n", bench_time);
	printf("%016lx\n", hash);

	//---------------------------------------------------------- bench B
	START_BENCH(start);
	for (size_t i = 0; i < test_count; i++) {
		for (size_t i = 0; i < KEYPOOL_SIZE; i++) {
			hash32 = murmurhash3_x86_32(
			    &random_keys[i], strlen(&random_keys[i]), seed);
			hash = mod_2n1(hash32, 11);
			// printf("%08lx : %s\n", hash32, &random_keys[i]);
		}
	}
	STOP_BENCH(start, stop, diff, bench_time);
	printf("bench murmurhash3_x86_32   \t: %f \n", bench_time);
	printf("%08lx %08lx\n", hash32, hash);

	//---------------------------------------------------------- bench C
	START_BENCH(start);
	for (size_t i = 0; i < test_count; i++) {
		for (size_t i = 0; i < KEYPOOL_SIZE; i++) {
			hash128 =
			    murmurhash3_x64(&random_keys[i], strlen(&random_keys[i]), seed);
				hash = mod_hash128(hash128, 11);
			// printf(
			//     "%016lx %016lx: %s\n", hash128.hi, hash128.lo,
			//     &random_keys[i]);
		}
	}
	STOP_BENCH(start, stop, diff, bench_time);
	printf("bench murmurhash3_x64   \t: %f \n", bench_time);
	printf("%016lx %016lx : %08lx\n", hash128.hi, hash128.lo, hash);

	//---------------------------------------------------------- bench D
	char *dummy_key;
	size_t hash_fimur = seed;
	START_BENCH(start);
	for (size_t i = 0; i < test_count; i++) {
		for (size_t i = 0; i < KEYPOOL_SIZE; i++) {
			dummy_key = &random_keys[i];
			while (*dummy_key) {
				hash_fimur *= 11400714819323198485llu * (*(dummy_key++) << 15);
				hash_fimur = (hash_fimur << 7) | (hash_fimur >> (32 - 7));
			}
			// printf("%016lx : %s\n", hash_xor, &random_keys[i]);
		}
	}
	STOP_BENCH(start, stop, diff, bench_time);
	printf("bench hash_fimur           \t: %f \n", bench_time);
	printf("%016lx\n", hash_fimur);

	//---------------------------------------------------------- bench E
	START_BENCH(start);
	for (size_t i = 0; i < test_count; i++) {
		for (size_t i = 0; i < KEYPOOL_SIZE; i++) {
			hash_fimur = hash_fimur_reduce(&random_keys[i], seed, 11);
		}
	}
	STOP_BENCH(start, stop, diff, bench_time);
	printf("bench hash_fimur_reduce        \t: %f \n", bench_time);
	printf("%016lx\n", hash_fimur);
	//---------------------------------------------------------- cleanup

	return 0;
}