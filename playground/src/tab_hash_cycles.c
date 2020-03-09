//------------------------------------------------------------------ SUMMARY ---
/**
 * Assess tab_hash number of CPU cycles per accesses for 8 bytes keys
 * Use Daniel Lemire fastrange demo
 *   See :
 * https://github.com/lemire/Code-used-on-Daniel-Lemire-s-blog/blob/master/2016/06/25/fastrange.c
 */

/**
 * Demonstrates how can map a 32-bit integer to a range faster than
 * a modulo reduction.
 * Assumes x64 processor.
 * gcc -O3 -o fastrange fastrange.c
 */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "hash_murmur3_nose.h"


/**
 * todo
 *   - [ ] Explore simd hash function
 *     + [ ] See : https://arxiv.org/pdf/1202.4961.pdf
 */
// #include <x86intrin.h>
// uint32_t hash_gfml_hmfast(uint32_t *m, uint32_t *p, uint32_t *endp)
// {
// 	__m128i z   = _mm_setzero_si128();
// 	__m128i sum = _mm_set_epi64x(0, *m);
// 	m += 4;

// 	__m128i t, u, t1, t2, ts, c1, c2;

// 	for(; p!= endp; m +=4, p+=4){
// 		t1 = _mm_load_si128((__m128i *)m);
// 		t2 = _mm_load_si128((__m128i *)p);
// 		ts = _mm_xor_si128(t1, t2);
// 		t = _mm_unpacklo_epi32(ts, z);
// 		c1 = _mm_clmulepi64_si128(t, t, 0x10);
// 		sum = _mm_xor_si128(c1, sum);
// 		u = _mm_unpackhi_epi32(ts, z);
// 		c2 = _mm_clmulepi64_si128(u, u, 0x10);
// 		sum = _mm_xor_si128(c2, sum);
// 	}

// 	// return reduce_barret(sum);
// 	return sum[0];
// }


#define RDTSC_START(cycles)                                                    \
	do {                                                                       \
		register unsigned cyc_high, cyc_low;                                   \
		__asm volatile(                                                        \
		    "cpuid\n\t"                                                        \
		    "rdtsc\n\t"                                                        \
		    "mov %%edx, %0\n\t"                                                \
		    "mov %%eax, %1\n\t"                                                \
		    : "=r"(cyc_high), "=r"(cyc_low)::"%rax", "%rbx", "%rcx", "%rdx");  \
		(cycles) = ((uint64_t)cyc_high << 32) | cyc_low;                       \
	} while (0)

#define RDTSC_FINAL(cycles)                                                    \
	do {                                                                       \
		register unsigned cyc_high, cyc_low;                                   \
		__asm volatile(                                                        \
		    "rdtscp\n\t"                                                       \
		    "mov %%edx, %0\n\t"                                                \
		    "mov %%eax, %1\n\t"                                                \
		    "cpuid\n\t"                                                        \
		    : "=r"(cyc_high), "=r"(cyc_low)::"%rax", "%rbx", "%rcx", "%rdx");  \
		(cycles) = ((uint64_t)cyc_high << 32) | cyc_low;                       \
	} while (0)

/*
 * Prints the best number of operations per cycle where
 * test is the function call, answer is the expected answer generated by
 * test, repeat is the number of times we should repeat and size is the
 * number of operations represented by test.
 */
#define BEST_TIME(test, answer, repeat, size)                                  \
	do {                                                                       \
		printf("%s: ", #test);                                                 \
		fflush(NULL);                                                          \
		uint64_t cycles_start, cycles_final, cycles_diff;                      \
		uint64_t min_diff = (uint64_t)-1;                                      \
		int wrong_answer  = 0;                                                 \
		for (int i = 0; i < repeat; i++) {                                     \
			__asm volatile("" ::: /* pretend to clobber */ "memory");          \
			RDTSC_START(cycles_start);                                         \
			if (test != answer)                                                \
				wrong_answer = 1;                                              \
			RDTSC_FINAL(cycles_final);                                         \
			cycles_diff = (cycles_final - cycles_start);                       \
			if (cycles_diff < min_diff)                                        \
				min_diff = cycles_diff;                                        \
		}                                                                      \
		uint64_t S         = (uint64_t)size;                                   \
		float cycle_per_op = (min_diff) / (float)S;                            \
		printf(" %.2f cycles per operation", cycle_per_op);                    \
		if (wrong_answer)                                                      \
			printf(" [ERROR]");                                                \
		printf("\n");                                                          \
		fflush(NULL);                                                          \
	} while (0)

size_t hash_tab(const unsigned char *key)
{
	static const size_t xor_table[] = {
	    27971, 29696, 1025,  26659, 62932, 5107,  35958, 44509, 34357, 7372,
	    12308, 36243, 62261, 65251, 39383, 10080, 28288, 11518, 1323,  44124,
	    52132, 23168, 3596,  31582, 8167,  36443, 11626, 18430, 30798, 43965,
	    59727, 1268,  28298, 35204, 51329, 27156, 58003, 29983, 8709,  47190,
	    60865, 53205, 51095, 62274, 14082, 12782, 62190, 39013, 15442, 30154,
	    30941, 2339,  42072, 39864, 45305, 52485, 49671, 19860, 55841, 28916,
	    35175, 2534,  35942, 28144, 23848, 29273, 59281, 9056,  30103, 60243,
	    25134, 22411, 13879, 57006, 12427, 32343, 4009,  37415, 45652, 51638,
	    18637, 8093,  11142, 7979,  34882, 59976, 42164, 59067, 8225,  53147,
	    13070, 29442, 10300, 51391, 16478, 27106, 7805,  54803, 55782, 64995,
	    17263, 27761, 25926, 48941, 3350,  3286,  44653, 27656, 8271,  45953,
	    10756, 39484, 64531, 6395,  8859,  62236, 23993, 28374, 40716, 44295,
	    43328, 36200, 18531, 6098,  1233,  4770,  2583,  6643,  10674, 36756,
	    11180, 25383, 34628, 59497, 48477, 47740, 60115, 17503, 28003, 10726,
	    57784, 29011, 22565, 32690, 35743, 16689, 4870,  21481, 3928,  59612,
	    51349, 30690, 23197, 33206, 55257, 44558, 16565, 51897, 17954, 28538,
	    2155,  60808, 13606, 34417, 2366,  34448, 16632, 9374,  48957, 50496,
	    41252, 26248, 52014, 53996, 5376,  61881, 51902, 50329, 46143, 51791,
	    24067, 37453, 32080, 41273, 45460, 63236, 42166, 18393, 40321, 55987,
	    62124, 57274, 29204, 10995, 5691,  12962, 15814, 174,   22691, 52354,
	    14838, 43033, 37401, 33756, 40194, 53728, 40672, 49446, 37287, 1115,
	    48396, 26441, 4633,  37936, 35133, 31609, 49588, 48314, 26113, 8879,
	    2437,  65176, 46792, 40390, 1328,  64648, 56361, 1794,  8309,  56310,
	    7282,  21274, 52611, 1793,  65286, 39254, 3863,  15019, 44753, 60445,
	    44700, 8710,  61930, 33656, 63895, 34451, 42755, 4603,  45978, 14290,
	    1180,  24418, 38082, 64862, 64087, 50749};

	size_t hash = 0;

	// while (*key) {
	for (size_t i = 0; i < 8; i++) {
		// printf("byte       = %u\n", *key);
		// printf("xor table  = %lu\n", xor_table[*key]);
		// printf("xored byte = %lu\n", xor_table[*key] ^ *key);
		hash ^= xor_table[(*key + hash) & 0xFF] ^ *key;
		// hash ^= xor_table[(unsigned char)(*key + hash)] ^ *key;
		key++;
		// printf("i          = %lu\n", i);
		// printf("hash       = %lu\n", hash);
	}

	return hash;
}

uint32_t hash_tab_access(uint32_t *z,
                         uint32_t N,
                         uint32_t *accesses,
                         uint32_t nmbr)
{
	(void)N;

	uint32_t sum = 0;
	for (uint32_t j = 0; j < nmbr; ++j) {
		// printf("%u\n", accesses[j]);
		// printf("%lu\n", hash_tab((unsigned char *)&accesses[j]));
		sum += z[hash_tab((unsigned char *)&accesses[j])];
		//  sum += z[accesses[j] & (N-1)] ;
	}
	return sum;
}

uint32_t murmurhash3_x86_32_mod_access(uint32_t *z,
                                       uint32_t N,
                                       uint32_t *accesses,
                                       uint32_t nmbr)
{
	uint32_t sum = 0;
	for (uint32_t j = 0; j < nmbr; ++j) {
		sum += z[murmurhash3_x86_32((void *)&accesses[j], 8, 31) % N];
	}
	return sum;
}

void demo(uint32_t N)
{
	printf("N = %d\n", N);
	uint32_t *z = malloc(N * sizeof(uint32_t));
	printf("size of z %lu \n", (N * sizeof(uint32_t)));

	for (uint32_t i = 0; i < N; ++i) {
		z[i] = rand(); // some rand. number
	}

	uint32_t nmbr      = 500;
	uint32_t *accesses = malloc(nmbr * sizeof(uint32_t));

	for (uint32_t i = 0; i < nmbr; ++i) {
		accesses[i] = rand(); // some rand. number
	}

	uint32_t expected1 = hash_tab_access(z, N, accesses, nmbr);
	uint32_t expected2 = murmurhash3_x86_32_mod_access(z, N, accesses, nmbr);

	BEST_TIME(hash_tab_access(z, N, accesses, nmbr), expected1, 1000, nmbr);
	BEST_TIME(murmurhash3_x86_32_mod_access(z, N, accesses, nmbr),
	          expected2,
	          1000,
	          nmbr);
	free(z);
	free(accesses);
}

//--------------------------------------------------------------------- MAIN ---
int main(void)
{

	// char key[256];
	printf("'8bytes!!' hash = %lu\n", hash_tab((unsigned char *)"8bytes!!"));
	demo(65536);
	// unsigned char code[] = "abd\1\2\3";
	// for (;;) {

	// 	fputs("\x1b[102m > \x1b[0m", stdout);
	// 	fgets(key, 255, stdin);

	// 	// if ((strcmp(key, "exit")) == 0) {
	// 	// 	break;
	// 	// }
	// 	// scanf("%s", key);
	// 	printf("%lu\n", hash_tab((unsigned char *)key));
	// }

	return 0;
}

