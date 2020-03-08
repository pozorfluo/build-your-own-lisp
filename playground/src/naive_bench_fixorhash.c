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
size_t xor_table[256];

inline size_t hash_tab(const unsigned char *key)
    __attribute__((const, always_inline));
//----------------------------------------------------------------- Function ---

inline size_t hash_tab(const unsigned char *key)
{
	// static const size_t xor_table[] = {
	//     27971, 29696, 1025,  26659, 62932, 5107,  35958, 44509, 34357, 7372,
	//     12308, 36243, 62261, 65251, 39383, 10080, 28288, 11518, 1323,  44124,
	//     52132, 23168, 3596,  31582, 8167,  36443, 11626, 18430, 30798, 43965,
	//     59727, 1268,  28298, 35204, 51329, 27156, 58003, 29983, 8709,  47190,
	//     60865, 53205, 51095, 62274, 14082, 12782, 62190, 39013, 15442, 30154,
	//     30941, 2339,  42072, 39864, 45305, 52485, 49671, 19860, 55841, 28916,
	//     35175, 2534,  35942, 28144, 23848, 29273, 59281, 9056,  30103, 60243,
	//     25134, 22411, 13879, 57006, 12427, 32343, 4009,  37415, 45652, 51638,
	//     18637, 8093,  11142, 7979,  34882, 59976, 42164, 59067, 8225,  53147,
	//     13070, 29442, 10300, 51391, 16478, 27106, 7805,  54803, 55782, 64995,
	//     17263, 27761, 25926, 48941, 3350,  3286,  44653, 27656, 8271,  45953,
	//     10756, 39484, 64531, 6395,  8859,  62236, 23993, 28374, 40716, 44295,
	//     43328, 36200, 18531, 6098,  1233,  4770,  2583,  6643,  10674, 36756,
	//     11180, 25383, 34628, 59497, 48477, 47740, 60115, 17503, 28003, 10726,
	// 57784, 29011, 22565, 32690, 35743, 16689, 4870,  21481, 3928,  59612,
	// 51349, 30690, 23197, 33206, 55257, 44558, 16565, 51897, 17954, 28538,
	// 2155,  60808, 13606, 34417, 2366,  34448, 16632, 9374,  48957, 50496,
	// 41252, 26248, 52014, 53996, 5376,  61881, 51902, 50329, 46143, 51791,
	// 24067, 37453, 32080, 41273, 45460, 63236, 42166, 18393, 40321, 55987,
	// 62124, 57274, 29204, 10995, 5691,  12962, 15814, 174,   22691, 52354,
	// 14838, 43033, 37401, 33756, 40194, 53728, 40672, 49446, 37287, 1115,
	// 48396, 26441, 4633,  37936, 35133, 31609, 49588, 48314, 26113, 8879,
	// 2437,  65176, 46792, 40390, 1328,  64648, 56361, 1794,  8309,  56310,
	// 7282,  21274, 52611, 1793,  65286, 39254, 3863,  15019, 44753, 60445,
	// 44700, 8710,  61930, 33656, 63895, 34451, 42755, 4603,  45978, 14290,
	// 1180,  24418, 38082, 64862, 64087, 50749};

	size_t hash = 0;
	// int i = xor_table[*key];
	// int i = 0;

	while (*key) {
		// hash ^= xor_table[(*key - i)& 0xFF] ^ *key;
		// hash ^= xor_table[(unsigned char)(*key + hash)] ^ *key;
		hash ^= xor_table[(*key + hash) & 0xFF] ^ *key;
		// hash ^= xor_table[(unsigned char)(*key - i)] ^ *key;
		// hash ^= (xor_table[*key] ^ *key) + i;
		key++;
		// i++;
		// i+=(unsigned char)xor_table[i];
		// i+=xor_table[i & 0xFF];
	}

	return hash;
}

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

	//------------------------------------------------------------ xor_table
	// setup

	size_t n = 1 << 24;
	for (size_t i = 0; i < 256; i++) {
		xor_table[i] = rand() % (n << 7);
	}
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

	//---------------------------------------------------------- bench F
	START_BENCH(start);
	for (size_t i = 0; i < test_count; i++) {
		for (size_t i = 0; i < KEYPOOL_SIZE; i++) {
			hash = hash_tab((unsigned char *)&random_keys[i]);
		}
	}
	STOP_BENCH(start, stop, diff, bench_time);
	printf("bench hash_tab   \t: %f \n", bench_time);
	printf("%016lx\n", hash);

	//---------------------------------------------------------- bench F
	START_BENCH(start);
	for (size_t i = 0; i < test_count; i++) {
		for (size_t i = 0; i < KEYPOOL_SIZE; i++) {
			hash = hash_tab((unsigned char *)&random_keys[i]);
		}
	}
	STOP_BENCH(start, stop, diff, bench_time);
	printf("bench hash_tab   \t: %f \n", bench_time);
	printf("%016lx\n", hash);

	//---------------------------------------------------------- bench F
	START_BENCH(start);
	for (size_t i = 0; i < test_count; i++) {
		for (size_t i = 0; i < KEYPOOL_SIZE; i++) {
			hash = hash_tab((unsigned char *)&random_keys[i]);
		}
	}
	STOP_BENCH(start, stop, diff, bench_time);
	printf("bench hash_tab   \t: %f \n", bench_time);
	printf("%016lx\n", hash);

	//---------------------------------------------------------- bench F
	START_BENCH(start);
	for (size_t i = 0; i < test_count; i++) {
		for (size_t i = 0; i < KEYPOOL_SIZE; i++) {
			hash = hash_tab((unsigned char *)&random_keys[i]);
		}
	}
	STOP_BENCH(start, stop, diff, bench_time);
	printf("bench hash_tab   \t: %f \n", bench_time);
	printf("%016lx\n", hash);
	//---------------------------------------------------------- bench E
	int cmp;
	START_BENCH(start);
	for (size_t i = 0; i < test_count; i++) {
		for (size_t i = 0; i < KEYPOOL_SIZE; i++) {
			cmp = strcmp(&random_keys[i], "hello yup");
		}
	}
	STOP_BENCH(start, stop, diff, bench_time);
	printf("bench str_cmp   \t: %f \n", bench_time);
	printf("%016x\n", cmp);
	//---------------------------------------------------------- bench E
	START_BENCH(start);
	for (size_t i = 0; i < test_count; i++) {
		for (size_t i = 0; i < KEYPOOL_SIZE; i++) {
			cmp = memcmp(&random_keys[i], "hello yup", strlen(&random_keys[i]));
		}
	}
	STOP_BENCH(start, stop, diff, bench_time);
	printf("bench str_len+memcmp \t: %f \n", bench_time);
	printf("%016x\n", cmp);
	//---------------------------------------------------------- bench E
	size_t src, dst;
	START_BENCH(start);
	for (size_t i = 0; i < test_count; i++) {
		for (size_t i = 0; i < KEYPOOL_SIZE - 1; i++) {
			src = strlen(&random_keys[i]);
			dst = strlen(&random_keys[i + 1]);
			cmp = memcmp(
			    &random_keys[i], &random_keys[i + 1], (src > dst) ? dst : src);
		}
	}
	STOP_BENCH(start, stop, diff, bench_time);
	printf("bench bounded str_len+memcmp \t: %f \n", bench_time);
	printf("%016x\n", cmp);
	//---------------------------------------------------------- cleanup

	return 0;
}