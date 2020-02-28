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
	stop = (float)clock() / CLOCKS_PER_SEC;                                    \
	diff = stop - start;                                                       \
	result = diff;

#else
#define LOOP_SIZE 0
#define START_BENCH(start)
#define STOP_BENCH(start, stop, diff, result)
#define BENCH(expression, loop, result)
#endif /* BENCHMARK */

//------------------------------------------------------------------- MACROS ---
#define ARRAY_LENGTH(array) (sizeof(array) / sizeof((array)[0]))

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
size_t hash_multiplicative(const char *key, const size_t seed)
    __attribute__((pure));

size_t hash_multiplicative(const char *key, const size_t seed)
{
	size_t hash = seed;

	while (*key) {
		hash = seed * hash + *key++;
	}

	return hash;
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
			// hash = hash_multiplicative(&random_keys[i], seed);
			// Shove strlen as seed to make comparison fair
			hash = hash_multiplicative(&random_keys[i], strlen(&random_keys[i]));
			// printf("%016lx : %s\n", hash, &random_keys[i]);
		}
	}
	STOP_BENCH(start, stop, diff, bench_time);
	printf("bench A   \t: %f \n", bench_time);
	printf("%016lx\n", hash);

	//---------------------------------------------------------- bench B
	START_BENCH(start);
	for (size_t i = 0; i < test_count; i++) {
		for (size_t i = 0; i < KEYPOOL_SIZE; i++) {
			hash32 = murmurhash3_x86_32(
			    &random_keys[i], strlen(&random_keys[i]), seed);
			// printf("%08lx : %s\n", hash32, &random_keys[i]);
		}
	}
	STOP_BENCH(start, stop, diff, bench_time);
	printf("bench B   \t: %f \n", bench_time);
	printf("%08lx\n", hash32);

	//---------------------------------------------------------- bench C
	START_BENCH(start);
	for (size_t i = 0; i < test_count; i++) {
		for (size_t i = 0; i < KEYPOOL_SIZE; i++) {
			hash128 =
			    murmurhash3_x64(&random_keys[i], strlen(&random_keys[i]), seed);
			// printf("%08lx : %s\n", hash32, &random_keys[i]);
		}
	}
	STOP_BENCH(start, stop, diff, bench_time);
	printf("bench B   \t: %f \n", bench_time);
	printf("%016lx %016lx\n", hash128.hi, hash128.lo);

	//---------------------------------------------------------- cleanup

	return 0;
}