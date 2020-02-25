//------------------------------------------------------------------ SUMMARY ---
/**
 * Adapted from :
 *
 *   MurmurHash3 was written by Austin Appleby, and is placed in the public
 *   domain. The author hereby disclaims copyright to this source code.
 *   Note - The x86 and x64 versions do _not_ produce the same results, as the
 *   algorithms are optimized for their respective platforms. You can still
 *   compile and run any of them on any platform, but your performance with the
 *   non-native version will be less than optimal.
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define LOOP_SIZE 10000000

//------------------------------------------------------------------- MACROS ---
#define ARRAY_LENGTH(array) (sizeof(array) / sizeof((array)[0]))

#define BIG_CONSTANT(x) (x##LLU)

#define ROTL32(_x, _r) (_x << _r) | (_x >> (32 - _r));

#define ROTL64(_x, _r) (_x << _r) | (_x >> (64 - _r));

/**
 * Block read
 * If your platform needs to do endian-swapping or can only handle aligned
 * reads, do the conversion here
 */
#define GETBLOCK(p, i) (p[i])

/**
 *  Finalize mix
 *  Force all bits of a hash block to avalanche
 */
#define FINALIZE_32(_h)                                                        \
	_h ^= _h >> 16;                                                            \
	_h *= 0x85ebca6b;                                                          \
	_h ^= _h >> 13;                                                            \
	_h *= 0xc2b2ae35;                                                          \
	_h ^= _h >> 16;

#define FINALIZE_64(_k)                                                        \
	_k ^= _k >> 33;                                                            \
	_k *= BIG_CONSTANT(0xff51afd7ed558ccd);                                    \
	_k ^= _k >> 33;                                                            \
	_k *= BIG_CONSTANT(0xc4ceb9fe1a85ec53);                                    \
	_k ^= _k >> 33;

//------------------------------------------------------------- DECLARATIONS ---
typedef struct Hash128_hilo {
	uint64_t hi;
	uint64_t lo;
} Hash128_hilo;

//----------------------------------------------------------------- Function ---
/**
 * Return 128-bit hash
 */
Hash128_hilo
murmurhash3_x64_hilo(const void *key, const int len, const uint32_t seed)
{
	const uint8_t *data = (const uint8_t *)key;
	const int nblocks   = len / 16;
	int i;

	uint64_t h1 = seed;
	uint64_t h2 = seed;

	uint64_t c1 = BIG_CONSTANT(0x87c37b91114253d5);
	uint64_t c2 = BIG_CONSTANT(0x4cf5ad432745937f);

	//------------------------------------------------------------- body
	const uint64_t *blocks = (const uint64_t *)(data);

	for (i = 0; i < nblocks; i++) {
		uint64_t k1 = GETBLOCK(blocks, i * 2 + 0);
		uint64_t k2 = GETBLOCK(blocks, i * 2 + 1);

		k1 *= c1;
		k1 = ROTL64(k1, 31);
		k1 *= c2;
		h1 ^= k1;

		h1 = ROTL64(h1, 27);
		h1 += h2;
		h1 = h1 * 5 + 0x52dce729;

		k2 *= c2;
		k2 = ROTL64(k2, 33);
		k2 *= c1;
		h2 ^= k2;

		h2 = ROTL64(h2, 31);
		h2 += h1;
		h2 = h2 * 5 + 0x38495ab5;
	}

	//------------------------------------------------------------- tail
	const uint8_t *tail = (const uint8_t *)(data + nblocks * 16);

	uint64_t k1 = 0;
	uint64_t k2 = 0;

	switch (len & 15) {
	case 15:
		k2 ^= (uint64_t)(tail[14]) << 48;
	case 14:
		k2 ^= (uint64_t)(tail[13]) << 40;
	case 13:
		k2 ^= (uint64_t)(tail[12]) << 32;
	case 12:
		k2 ^= (uint64_t)(tail[11]) << 24;
	case 11:
		k2 ^= (uint64_t)(tail[10]) << 16;
	case 10:
		k2 ^= (uint64_t)(tail[9]) << 8;
	case 9:
		k2 ^= (uint64_t)(tail[8]) << 0;
		k2 *= c2;
		k2 = ROTL64(k2, 33);
		k2 *= c1;
		h2 ^= k2;

	case 8:
		k1 ^= (uint64_t)(tail[7]) << 56;
	case 7:
		k1 ^= (uint64_t)(tail[6]) << 48;
	case 6:
		k1 ^= (uint64_t)(tail[5]) << 40;
	case 5:
		k1 ^= (uint64_t)(tail[4]) << 32;
	case 4:
		k1 ^= (uint64_t)(tail[3]) << 24;
	case 3:
		k1 ^= (uint64_t)(tail[2]) << 16;
	case 2:
		k1 ^= (uint64_t)(tail[1]) << 8;
	case 1:
		k1 ^= (uint64_t)(tail[0]) << 0;
		k1 *= c1;
		k1 = ROTL64(k1, 31);
		k1 *= c2;
		h1 ^= k1;
	};

	//----------------------------------------------------- finalization
	h1 ^= len;
	h2 ^= len;

	h1 += h2;
	h2 += h1;

	FINALIZE_64(h1);
	FINALIZE_64(h2);

	h1 += h2;
	h2 += h1;

	// ((uint64_t *)out)[0] = h1;
	// ((uint64_t *)out)[1] = h2;
	Hash128_hilo hash;
	hash.hi = h1;
	hash.lo = h2;
	return hash;
}

//--------------------------------------------------------------------- MAIN ---
int main(void)
{
	uint32_t seed = 42;

	//----------------------------------------------------------- hash tests
	const char *keys[] = {
	    "too",  "top",  "tor",  "tpp",  "a000", "a001", "a002", "a003", "a004",
	    "a005", "a006", "a007", "a008", "a009", "a010", "a",    "aa",   "aaa",
	    "add",  "sub",  "mul",  "div",  "mod",  "pow",  "max",  "min",  "+",
	    "-",    "*",    "/",    "%",    "^",    ">",    "<",    "head", "tail",
	    "list", "init", "eval", "join", "cons", "len",  "def",  "env"};

	//---------------------------------------------- bench return struct
	Hash128_hilo hash_hilo;

	for (size_t n = 0; n < LOOP_SIZE; n++) {
		for (size_t i = 0; i < ARRAY_LENGTH(keys); i++) {
			hash_hilo = murmurhash3_x64_hilo(keys[i], strlen(keys[i]), seed);

		}
	}

	printf("x64_128: %016lx %016lx\n", hash_hilo.hi, hash_hilo.lo);

	return 0;
}