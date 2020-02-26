//------------------------------------------------------------------ SUMMARY ---
/**
 * Adapted from :
 *   https://github.com/aappleby/smhasher/blob/master/src/MurmurHash3.cpp
 *
 *   MurmurHash3 was written by Austin Appleby, and is placed in the public
 *   domain. The author hereby disclaims copyright to this source code.
 *
 *   Note - The x86 and x64 versions do _not_ produce the same results, as the
 *   algorithms are optimized for their respective platforms. You can still
 *   compile and run any of them on any platform, but your performance with the
 *   non-native version will be less than optimal.
 */

#include <stdio.h>
/**
 * todo
*   - [ ] Move to separate example
 */ 
// #include <string.h>

#include "hash_murmur3_nose.h"

//------------------------------------------------------------------- MACROS ---
/**
 * todo
 *   - [ ] Move to separate example
 */ 
// #define EXIT_SUCCESS 0
// #define EXIT_FAILURE 1

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
	_k *= 0xff51afd7ed558ccdull;                                               \
	_k ^= _k >> 33;                                                            \
	_k *= 0xc4ceb9fe1a85ec53ull;                                               \
	_k ^= _k >> 33;

//------------------------------------------------------------- DECLARATIONS ---
// typedef struct Hash128 {
// 	uint64_t hi;
// 	uint64_t lo;
// } Hash128;

//----------------------------------------------------------------- Function ---
/**
 * Return 32 bit hash
 */
uint32_t murmurhash3_x86_32(const void *key,
                            const size_t len,
                            const uint32_t seed)
{
	const uint32_t c1 = 0xcc9e2d51;
	const uint32_t c2 = 0x1b873593;
	const uint32_t r1 = 15;
	const uint32_t r2 = 13;
	const uint32_t m  = 5;
	const uint32_t n  = 0xe6546b64;

	const uint8_t *data  = (const uint8_t *)key;
	const size_t nblocks = len / 4;

	uint32_t hash = seed;

	//------------------------------------------------------------- body
	const uint32_t *blocks = (const uint32_t *)(data + nblocks * 4);

	for (int i = -nblocks; i; i++) {
		uint32_t k = GETBLOCK(blocks, i);
		k *= c1;
		k = ROTL32(k, r1);
		k *= c2;

		hash ^= k;
		hash = ROTL32(hash, r2);
		hash = hash * m + n;
	}

	//------------------------------------------------------------- tail
	const uint8_t *tail = (const uint8_t *)(data + nblocks * 4);

	uint32_t k = 0;

	switch (len & 3) {
	case 3:
		k ^= tail[2] << 16;
	case 2:
		k ^= tail[1] << 8;
	case 1:
		k ^= tail[0];
		k *= c1;
		k = ROTL32(k, r1);
		k *= c2;
		hash ^= k;
	};

	//----------------------------------------------------- finalization
	hash ^= len;

	FINALIZE_32(hash);

	return hash;
}

//----------------------------------------------------------------- Function ---
/**
 * Return 128-bit hash
 */
Hash128 murmurhash3_x86_128(const void *key,
                            const size_t len,
                            const uint32_t seed)
{
	const uint32_t c1 = 0x239b961b;
	const uint32_t c2 = 0xab0e9789;
	const uint32_t c3 = 0x38b34ae5;
	const uint32_t c4 = 0xa1e38b93;

	const uint32_t r1 = 15;
	const uint32_t r2 = 19;
	const uint32_t r3 = 16;
	const uint32_t r4 = 17;
	const uint32_t r5 = 18;
	const uint32_t r6 = 13;

	const uint32_t m = 5;
	const uint32_t n = 0x561ccd1b;
	const uint32_t o = 0x0bcaa747;
	const uint32_t p = 0x96cd1c35;
	const uint32_t q = 0x32ac3b17;

	const uint8_t *data = (const uint8_t *)key;
	const int nblocks   = len / 16;

	uint32_t h1 = seed;
	uint32_t h2 = seed;
	uint32_t h3 = seed;
	uint32_t h4 = seed;

	//------------------------------------------------------------- body
	const uint32_t *blocks = (const uint32_t *)(data + nblocks * 16);

	for (int i = -nblocks; i; i++) {
		uint32_t k1 = GETBLOCK(blocks, i * 4 + 0);
		uint32_t k2 = GETBLOCK(blocks, i * 4 + 1);
		uint32_t k3 = GETBLOCK(blocks, i * 4 + 2);
		uint32_t k4 = GETBLOCK(blocks, i * 4 + 3);

		k1 *= c1;
		k1 = ROTL32(k1, r1);
		k1 *= c2;
		h1 ^= k1;

		h1 = ROTL32(h1, r2);
		h1 += h2;
		h1 = h1 * m + n;

		k2 *= c2;
		k2 = ROTL32(k2, r3);
		k2 *= c3;
		h2 ^= k2;

		h2 = ROTL32(h2, r4);
		h2 += h3;
		h2 = h2 * m + o;

		k3 *= c3;
		k3 = ROTL32(k3, r4);
		k3 *= c4;
		h3 ^= k3;

		h3 = ROTL32(h3, r1);
		h3 += h4;
		h3 = h3 * m + p;

		k4 *= c4;
		k4 = ROTL32(k4, r5);
		k4 *= c1;
		h4 ^= k4;

		h4 = ROTL32(h4, r6);
		h4 += h1;
		h4 = h4 * m + q;
	}

	//------------------------------------------------------------- tail
	const uint8_t *tail = (const uint8_t *)(data + nblocks * 16);

	uint32_t k1 = 0;
	uint32_t k2 = 0;
	uint32_t k3 = 0;
	uint32_t k4 = 0;

	switch (len & 15) {
	case 15:
		k4 ^= tail[14] << 16;
	case 14:
		k4 ^= tail[13] << 8;
	case 13:
		k4 ^= tail[12] << 0;
		k4 *= c4;
		k4 = ROTL32(k4, r5);
		k4 *= c1;
		h4 ^= k4;

	case 12:
		k3 ^= tail[11] << 24;
	case 11:
		k3 ^= tail[10] << 16;
	case 10:
		k3 ^= tail[9] << 8;
	case 9:
		k3 ^= tail[8] << 0;
		k3 *= c3;
		k3 = ROTL32(k3, r4);
		k3 *= c4;
		h3 ^= k3;

	case 8:
		k2 ^= tail[7] << 24;
	case 7:
		k2 ^= tail[6] << 16;
	case 6:
		k2 ^= tail[5] << 8;
	case 5:
		k2 ^= tail[4] << 0;
		k2 *= c2;
		k2 = ROTL32(k2, r3);
		k2 *= c3;
		h2 ^= k2;

	case 4:
		k1 ^= tail[3] << 24;
	case 3:
		k1 ^= tail[2] << 16;
	case 2:
		k1 ^= tail[1] << 8;
	case 1:
		k1 ^= tail[0] << 0;
		k1 *= c1;
		k1 = ROTL32(k1, r1);
		k1 *= c2;
		h1 ^= k1;
	};

	//----------------------------------------------------- finalization
	h1 ^= len;
	h2 ^= len;
	h3 ^= len;
	h4 ^= len;

	h1 += h2;
	h1 += h3;
	h1 += h4;
	h2 += h1;
	h3 += h1;
	h4 += h1;

	FINALIZE_32(h1);
	FINALIZE_32(h2);
	FINALIZE_32(h3);
	FINALIZE_32(h4);

	h1 += h2;
	h1 += h3;
	h1 += h4;
	h2 += h1;
	h3 += h1;
	h4 += h1;

	Hash128 hash = {((uint64_t)h1 << 32) + h2, ((uint64_t)h3 << 32) + h4};

	return hash;
}

//----------------------------------------------------------------- Function ---
/**
 * Return 128-bit hash
 */
Hash128 murmurhash3_x64(const void *key, const size_t len, const uint32_t seed)
{
	const uint64_t c1 = 0x87c37b91114253d5ull;
	const uint64_t c2 = 0x4cf5ad432745937full;

	const uint32_t r1 = 31;
	const uint32_t r2 = 27;
	const uint32_t r3 = 33;

	const uint32_t m = 5;
	const uint32_t n = 0x52dce729;
	const uint32_t o = 0x38495ab5;

	const uint8_t *data  = (const uint8_t *)key;
	const size_t nblocks = len / 16;

	uint64_t h1 = seed;
	uint64_t h2 = seed;

	//------------------------------------------------------------- body
	const uint64_t *blocks = (const uint64_t *)(data);

	for (size_t i = 0; i < nblocks; i++) {
		uint64_t k1 = GETBLOCK(blocks, i * 2 + 0);
		uint64_t k2 = GETBLOCK(blocks, i * 2 + 1);

		k1 *= c1;
		k1 = ROTL64(k1, r1);
		k1 *= c2;
		h1 ^= k1;

		h1 = ROTL64(h1, r2);
		h1 += h2;
		h1 = h1 * m + n;

		k2 *= c2;
		k2 = ROTL64(k2, r3);
		k2 *= c1;
		h2 ^= k2;

		h2 = ROTL64(h2, r1);
		h2 += h1;
		h2 = h2 * m + o;
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
		k2 = ROTL64(k2, r3);
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
		k1 = ROTL64(k1, r1);
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

	Hash128 hash = {h1, h2};

	return hash;
}

//--------------------------------------------------------------------- MAIN ---
/**
 * todo
 *   - [ ] Move to separate example
 */ 
// int main(int argc, char **argv)
// {
// 	//------------------------------------------------------------ usage
// 	if (argc != 2) {
// 		printf("Usage : %s \"string to hash\"\n", argv[0]);
// 		return EXIT_FAILURE;
// 	}

// 	//------------------------------------------------------------- work
// 	uint32_t seed = 47;
// 	uint32_t hash32;
// 	Hash128 hash;

// 	printf("size of uint32_t : %lu bytes\n", sizeof(seed));
// 	printf("size of Hash128  : %lu bytes\n\n", sizeof(hash));

// 	printf("Input   : \"%s\"\n", argv[1]);

// 	hash32 = murmurhash3_x86_32(argv[1], strlen(argv[1]), seed);
// 	printf("x86_32  : %08x\n", hash32);

// 	hash = murmurhash3_x86_128(argv[1], strlen(argv[1]), seed);
// 	printf("x86_128 : %016lx %016lx\n", hash.hi, hash.lo);

// 	hash = murmurhash3_x64(argv[1], strlen(argv[1]), seed);
// 	printf("x64_128 : %016lx %016lx\n", hash.hi, hash.lo);

// 	return EXIT_SUCCESS;
// }