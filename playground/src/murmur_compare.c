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

#define BENCHMARK
#include <fcntl.h>
#include <unistd.h>

/*
 * ------------------------------------------------------------- DEBUG MACRO ---
 */
#ifdef BENCHMARK
#include <time.h>

#define LOOP_SIZE 10000000

#define START_BENCH(start) start = (float)clock() / CLOCKS_PER_SEC

#define STOP_BENCH(start, stop, diff, result)                                  \
	stop = (float)clock() / CLOCKS_PER_SEC;                                    \
	diff = stop - start;                                                       \
	printf("%.8f\n", diff);                                                    \
	result = diff;

#else
#define LOOP_SIZE 0
#define START_BENCH(start)
#define STOP_BENCH(start, stop, diff, result)
#define BENCH(expression, loop, result)
#endif
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
typedef struct Hash128_4x32 {
	uint32_t chunks[4];
} Hash128_4x32;

typedef struct Hash128_hilo {
	uint64_t hi;
	uint64_t lo;
} Hash128_hilo;

typedef struct Hash128 {
	uint64_t chunks[2];
} Hash128;
//----------------------------------------------------------------- Function ---
/**
 * Return 32 bit hash
 */
void murmurhash3_x86_32(const void *key, int len, uint32_t seed, void *out)
{
	const uint8_t *data = (const uint8_t *)key;
	const int nblocks   = len / 4;
	int i;

	uint32_t h1 = seed;

	uint32_t c1 = 0xcc9e2d51;
	uint32_t c2 = 0x1b873593;

	//------------------------------------------------------------- body
	const uint32_t *blocks = (const uint32_t *)(data + nblocks * 4);

	for (i = -nblocks; i; i++) {
		uint32_t k1 = GETBLOCK(blocks, i);

		k1 *= c1;
		k1 = ROTL32(k1, 15);
		k1 *= c2;

		h1 ^= k1;
		h1 = ROTL32(h1, 13);
		h1 = h1 * 5 + 0xe6546b64;
	}

	//------------------------------------------------------------- tail
	const uint8_t *tail = (const uint8_t *)(data + nblocks * 4);

	uint32_t k1 = 0;

	switch (len & 3) {
	case 3:
		k1 ^= tail[2] << 16;
	case 2:
		k1 ^= tail[1] << 8;
	case 1:
		k1 ^= tail[0];
		k1 *= c1;
		k1 = ROTL32(k1, 15);
		k1 *= c2;
		h1 ^= k1;
	};

	//----------------------------------------------------- finalization
	h1 ^= len;

	FINALIZE_32(h1);

	*(uint32_t *)out = h1;
}

//----------------------------------------------------------------- Function ---
/**
 * Return 128-bit hash
 */
void murmurhash3_x86_128(const void *key,
                         const int len,
                         const uint32_t seed,
                         void *out)
{
	const uint8_t *data = (const uint8_t *)key;
	const int nblocks   = len / 16;
	int i;

	uint32_t h1 = seed;
	uint32_t h2 = seed;
	uint32_t h3 = seed;
	uint32_t h4 = seed;

	uint32_t c1 = 0x239b961b;
	uint32_t c2 = 0xab0e9789;
	uint32_t c3 = 0x38b34ae5;
	uint32_t c4 = 0xa1e38b93;

	//------------------------------------------------------------- body
	const uint32_t *blocks = (const uint32_t *)(data + nblocks * 16);

	for (i = -nblocks; i; i++) {
		uint32_t k1 = GETBLOCK(blocks, i * 4 + 0);
		uint32_t k2 = GETBLOCK(blocks, i * 4 + 1);
		uint32_t k3 = GETBLOCK(blocks, i * 4 + 2);
		uint32_t k4 = GETBLOCK(blocks, i * 4 + 3);

		k1 *= c1;
		k1 = ROTL32(k1, 15);
		k1 *= c2;
		h1 ^= k1;

		h1 = ROTL32(h1, 19);
		h1 += h2;
		h1 = h1 * 5 + 0x561ccd1b;

		k2 *= c2;
		k2 = ROTL32(k2, 16);
		k2 *= c3;
		h2 ^= k2;

		h2 = ROTL32(h2, 17);
		h2 += h3;
		h2 = h2 * 5 + 0x0bcaa747;

		k3 *= c3;
		k3 = ROTL32(k3, 17);
		k3 *= c4;
		h3 ^= k3;

		h3 = ROTL32(h3, 15);
		h3 += h4;
		h3 = h3 * 5 + 0x96cd1c35;

		k4 *= c4;
		k4 = ROTL32(k4, 18);
		k4 *= c1;
		h4 ^= k4;

		h4 = ROTL32(h4, 13);
		h4 += h1;
		h4 = h4 * 5 + 0x32ac3b17;
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
		k4 = ROTL32(k4, 18);
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
		k3 = ROTL32(k3, 17);
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
		k2 = ROTL32(k2, 16);
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
		k1 = ROTL32(k1, 15);
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

	((uint32_t *)out)[0] = h1;
	((uint32_t *)out)[1] = h2;
	((uint32_t *)out)[2] = h3;
	((uint32_t *)out)[3] = h4;
}

//----------------------------------------------------------------- Function ---
/**
 * Return 128-bit hash
 */
Hash128 murmurhash3_x64_128(const void *key, const int len, const uint32_t seed)
//  void *out)
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
	Hash128 hash;
	hash.chunks[0] = h1;
	hash.chunks[1] = h2;
	return hash;
}

//----------------------------------------------------------------- Function ---
/**
 * Return 128-bit hash
 */
// Hash128_hilo
// murmurhash3_x64_hilo(const void *key, const size_t len, const uint32_t seed)
// {
// 	const uint8_t *data = (const uint8_t *)key;
// 	const size_t nblocks   = len / 16;

// 	uint64_t h1 = seed;
// 	uint64_t h2 = seed;

// 	uint64_t c1 = 0x87c37b91114253d5ull;
// 	uint64_t c2 = 0x4cf5ad432745937full;

// 	//------------------------------------------------------------- body
// 	const uint64_t *blocks = (const uint64_t *)(data);

// 	for (size_t i = 0; i < nblocks; i++) {
// 		uint64_t k1 = GETBLOCK(blocks, i * 2 + 0);
// 		uint64_t k2 = GETBLOCK(blocks, i * 2 + 1);

// 		k1 *= c1;
// 		k1 = ROTL64(k1, 31);
// 		k1 *= c2;
// 		h1 ^= k1;

// 		h1 = ROTL64(h1, 27);
// 		h1 += h2;
// 		h1 = h1 * 5 + 0x52dce729;

// 		k2 *= c2;
// 		k2 = ROTL64(k2, 33);
// 		k2 *= c1;
// 		h2 ^= k2;

// 		h2 = ROTL64(h2, 31);
// 		h2 += h1;
// 		h2 = h2 * 5 + 0x38495ab5;
// 	}

// 	//------------------------------------------------------------- tail
// 	const uint8_t *tail = (const uint8_t *)(data + nblocks * 16);

// 	uint64_t k1 = 0;
// 	uint64_t k2 = 0;

// 	switch (len & 15) {
// 	case 15:
// 		k2 ^= (uint64_t)(tail[14]) << 48;
// 	case 14:
// 		k2 ^= (uint64_t)(tail[13]) << 40;
// 	case 13:
// 		k2 ^= (uint64_t)(tail[12]) << 32;
// 	case 12:
// 		k2 ^= (uint64_t)(tail[11]) << 24;
// 	case 11:
// 		k2 ^= (uint64_t)(tail[10]) << 16;
// 	case 10:
// 		k2 ^= (uint64_t)(tail[9]) << 8;
// 	case 9:
// 		k2 ^= (uint64_t)(tail[8]) << 0;
// 		k2 *= c2;
// 		k2 = ROTL64(k2, 33);
// 		k2 *= c1;
// 		h2 ^= k2;

// 	case 8:
// 		k1 ^= (uint64_t)(tail[7]) << 56;
// 	case 7:
// 		k1 ^= (uint64_t)(tail[6]) << 48;
// 	case 6:
// 		k1 ^= (uint64_t)(tail[5]) << 40;
// 	case 5:
// 		k1 ^= (uint64_t)(tail[4]) << 32;
// 	case 4:
// 		k1 ^= (uint64_t)(tail[3]) << 24;
// 	case 3:
// 		k1 ^= (uint64_t)(tail[2]) << 16;
// 	case 2:
// 		k1 ^= (uint64_t)(tail[1]) << 8;
// 	case 1:
// 		k1 ^= (uint64_t)(tail[0]) << 0;
// 		k1 *= c1;
// 		k1 = ROTL64(k1, 31);
// 		k1 *= c2;
// 		h1 ^= k1;
// 	};

// 	//----------------------------------------------------- finalization
// 	h1 ^= len;
// 	h2 ^= len;

// 	h1 += h2;
// 	h2 += h1;

// 	FINALIZE_64(h1);
// 	FINALIZE_64(h2);

// 	h1 += h2;
// 	h2 += h1;

// 	// ((uint64_t *)out)[0] = h1;
// 	// ((uint64_t *)out)[1] = h2;
// 	Hash128_hilo hash;
// 	hash.hi = h1;
// 	hash.lo = h2;
// 	return hash;
// }
// #define X64_128_C1 0x87c37b91114253d5ull
// #define X64_128_C2 0x4cf5ad432745937full

Hash128_hilo
murmurhash3_x64_hilo(const void *key, const size_t len, const uint32_t seed)
// Hash128 murmurhash3_x64(const void *key, const size_t len, const uint32_t
// seed)
{
	const uint8_t *data  = (const uint8_t *)key;
	const size_t nblocks = len / 16;

	uint64_t h1 = seed;
	uint64_t h2 = seed;

	const uint64_t X64_128_C1 = 0x87c37b91114253d5ull;
	const uint64_t X64_128_C2 = 0x4cf5ad432745937full;

	//------------------------------------------------------------- body
	const uint64_t *blocks = (const uint64_t *)(data);

	for (size_t i = 0; i < nblocks; i++) {
		uint64_t k1 = GETBLOCK(blocks, i * 2 + 0);
		uint64_t k2 = GETBLOCK(blocks, i * 2 + 1);

		k1 *= X64_128_C1;
		k1 = ROTL64(k1, 31);
		k1 *= X64_128_C2;
		h1 ^= k1;

		h1 = ROTL64(h1, 27);
		h1 += h2;
		h1 = h1 * 5 + 0x52dce729;

		k2 *= X64_128_C2;
		k2 = ROTL64(k2, 33);
		k2 *= X64_128_C1;
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
		k2 *= X64_128_C2;
		k2 = ROTL64(k2, 33);
		k2 *= X64_128_C1;
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
		k1 *= X64_128_C1;
		k1 = ROTL64(k1, 31);
		k1 *= X64_128_C2;
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

	Hash128_hilo hash = {h1, h2};

	return hash;
}
//----------------------------------------------------------------- Function ---
/**
 * Return 128-bit hash
 */
void murmurhash3_x64_128_og(const void *key,
                            const int len,
                            const uint32_t seed,
                            void *out)
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

	((uint64_t *)out)[0] = h1;
	((uint64_t *)out)[1] = h2;
}

//----------------------------------------------------------------- Function ---
/**
 * Return 128-bit hash
 */
Hash128_hilo murmurhash3_x86_128_const(const void *key,
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

	Hash128_hilo hash = {((uint64_t)h1 << 32) + h2, ((uint64_t)h3 << 32) + h4};

	return hash;
}

//----------------------------------------------------------------- Function ---
/**
 * Return 128-bit hash
 */
Hash128_hilo
murmurhash3_x64_128_const(const void *key, const size_t len, const uint32_t seed)
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

	Hash128_hilo hash = {h1, h2};

	return hash;
}

//----------------------------------------------------------------- Function ---
/**
 * Returns a multiplicative style hash for given key
 *
 * see : Linear congruential generator
 */
size_t hash_multiplicative(const char *key, const size_t seed)
{
	size_t hash = seed;
	// const size_t length = strlen(key);

	// for (size_t i = 0; i < length; i++) {
	// 	hash = seed * hash + key[i];
	// }
	while (*key) {
		hash = seed * hash + *key++;
	}

	return hash;
}
//--------------------------------------------------------------------- MAIN ---
int main(void)
{
	uint32_t seed = 42;
	//-------------------------------------------------------- benchmark
	// int backup, new;
	// fflush(stdout);
	// backup = dup(1);
	// new    = open("/dev/null", O_WRONLY);
	// dup2(new, 1);
	// close(new);

	//----------------------------------------------------------- hash tests
	const char *keys[] = {
	    "too",
	    "top",
	    "tor",
	    "tpp",
	    "a000",
	    "a001",
	    "a002",
	    "a003",
	    "a004",
	    "a005",
	    "a006",
	    "a007",
	    "a008",
	    "a009",
	    "a010",
	    "a",
	    "aa",
	    "aaa",
	    "add",
	    "sub",
	    "mul",
	    "div",
	    "mod",
	    "pow",
	    "max",
	    "min",
	    "+",
	    "-",
	    "*",
	    "/",
	    "%",
	    "^",
	    ">",
	    "<",
	    "head",
	    "tail",
	    "list",
	    "init",
	    "eval",
	    "join",
	    "cons",
	    "len",
	    "def",
	    "env",
	    "hello, world !",
	    "azjelaizje alkazje lkazjeal kzjelakje ",
	    "azjelaizje alkazje lkazjeal kzjelakjeddddddddddddddddd ",
	    "azjelaizje alkazje lkazjeal kzjeqfqsfqsfqsflakje ",
	    "azjelaizje alkazje lkazjeal kzjelakgzegzegje ",
	    "azjelaizje alkazje lkazjeal kzjelak65656565+5+5je ",
	    "black and yellow black and yellow black and yellow",
	    "alzkejlkjlkjlkaalkjlkjze aklz jeazlkej alzkej"};

	// float results[TEST_COUNT];

	// BENCH(murmurhash3_x64_128, LOOP_SIZE, results[0]);
	float start, stop, diff, result;

	//---------------------------------------------- bench return struct
	Hash128 hash128;
	START_BENCH(start);

	for (size_t n = 0; n < LOOP_SIZE; n++) {
		for (size_t i = 0; i < ARRAY_LENGTH(keys); i++) {
			hash128 = murmurhash3_x64_128(keys[i], strlen(keys[i]), seed);
		}
	}

	STOP_BENCH(start, stop, diff, result);
	printf("murmurhash3_x64_128   \t: %f \n", result);

	//---------------------------------------------- bench return struct
	Hash128_hilo hash_hilo;
	START_BENCH(start);

	for (size_t n = 0; n < LOOP_SIZE; n++) {
		for (size_t i = 0; i < ARRAY_LENGTH(keys); i++) {
			hash_hilo = murmurhash3_x64_hilo(keys[i], strlen(keys[i]), seed);
			// printf("x64_128: %016lx %016lx\n",
			//        hash_hilo.hi,
			//        hash_hilo.lo);
		}
	}

	STOP_BENCH(start, stop, diff, result);
	printf("murmurhash3_x64_hilo   \t: %f \n", result);

	//---------------------------------------------- bench return struct
	Hash128_hilo hash_const;
	START_BENCH(start);

	for (size_t n = 0; n < LOOP_SIZE; n++) {
		for (size_t i = 0; i < ARRAY_LENGTH(keys); i++) {
			hash_const =
			    murmurhash3_x86_128_const(keys[i], strlen(keys[i]), seed);
			// printf("x64_128: %016lx %016lx\n",
			//        hash_hilo.hi,
			//        hash_hilo.lo);
		}
	}

	STOP_BENCH(start, stop, diff, result);
	printf("murmurhash3_x86_128_const   \t: %f \n", result);

	//--------------------------------------------------------- bench og
	uint32_t hash[4];
	START_BENCH(start);

	for (size_t n = 0; n < LOOP_SIZE; n++) {
		for (size_t i = 0; i < ARRAY_LENGTH(keys); i++) {
			murmurhash3_x64_128_og(keys[i], strlen(keys[i]), seed, hash);
		}
	}

	STOP_BENCH(start, stop, diff, result);
	printf("murmurhash3_x64_128_og   \t: %f \n", result);

	//---------------------------------------------- bench return struct
	Hash128_hilo hash_x64const;
	START_BENCH(start);

	for (size_t n = 0; n < LOOP_SIZE; n++) {
		for (size_t i = 0; i < ARRAY_LENGTH(keys); i++) {
			hash_x64const =
			    murmurhash3_x64_128_const(keys[i], strlen(keys[i]), seed);
			// printf("x64_128: %016lx %016lx\n",
			//        hash_hilo.hi,
			//        hash_hilo.lo);
		}
	}

	STOP_BENCH(start, stop, diff, result);
	printf("murmurhash3_x64_128_const   \t: %f \n", result);

	//---------------------------------------- bench hash_multiplicative
	size_t hash_mult;
	START_BENCH(start);

	for (size_t n = 0; n < LOOP_SIZE; n++) {
		for (size_t i = 0; i < ARRAY_LENGTH(keys); i++) {
			hash_mult =
			    hash_multiplicative(keys[i], seed);
			// printf("x64_128: %016lx %016lx\n",
			//        hash_hilo.hi,
			//        hash_hilo.lo);
		}
	}

	STOP_BENCH(start, stop, diff, result);
	printf("hash_mult   \t: %f \n", result);

	// print last hash of each benchmark to make sure the whole loop is not
	// discarded by compiler ?
	printf(
	    "x64_128: %08x %08x %08x %08x\n", hash[0], hash[1], hash[2], hash[3]);

	printf("x64_128: %016lx %016lx\n", hash128.chunks[0], hash128.chunks[1]);

	printf("x64_128: %016lx %016lx\n", hash_hilo.hi, hash_hilo.lo);
	printf("x86_128: %016lx %016lx\n", hash_const.hi, hash_const.lo);
	printf("x64_128: %016lx %016lx\n", hash_x64const.hi, hash_x64const.lo);
	printf("hash_mult: %016lx\n", hash_mult);
	return 0;
}