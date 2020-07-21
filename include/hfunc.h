//------------------------------------------------------------------ SUMMARY ---
/**
 * Collection of hash functions and hash reductions.
 *
 * todo Consider testing Lemire's fast range.
 *        See
 * https://lemire.me/blog/2016/06/27/a-fast-alternative-to-the-modulo-reduction/
 *        See https://github.com/lemire/fastrange/blob/master/fastrange.h
 */

#ifndef HFUNC_H
#define HFUNC_H

#include <stddef.h> /* size_t */

//------------------------------------------------------------ CONFIGURATION ---

#ifdef TEST_REGISTER
#define HFUNC_REGISTER register
#else
#define HFUNC_REGISTER
#endif /* TEST_REGISTER */
//------------------------------------------------------------ MAGIC NUMBERS ---
#define HFIBO 11400714819323198485llu
#define HSEED 11400714819323198485llu
//------------------------------------------------------------- DECLARATIONS ---
static inline size_t hash_perl(const char *const key, size_t length)
    __attribute__((pure, always_inline));

static inline size_t hash_multiplicative(const char *const key, size_t length)
    __attribute__((pure, always_inline));

static inline size_t hash_djb2(const char *const key, size_t length)
    __attribute__((pure, always_inline));

static inline size_t hash_djb2_alt(const char *const key, size_t length)
    __attribute__((pure, always_inline));

static inline size_t hash_kh_str(const char *const key, size_t length)
    __attribute__((pure, always_inline));

static inline size_t hash_fnv1a(const char *const key, size_t length)
    __attribute__((pure, always_inline));

static inline size_t reduce_fibo(const size_t hash, const size_t shift)
    __attribute__((const, always_inline));
//----------------------------------------------------------------- Function ---
/**
 * Reduce a given hash to a packed hmap index and meta_byte.
 *
 * also can be used as :
 *
 * Compute a hash, multiplicative style with a fibonacci derived constant, for a
 * given numeric key.
 *
 *   -> packed hmap index and meta_byte
 *
 * note
 *   This assumes that the max size possibly requested for hmap is 2^57 or
 * 2^( 64 - 7 bits of "extra hash" required for meta_byte ).
 *
 *   i.e., 144,115,188,075,855,872 max advertised capacity.
 */
static inline size_t reduce_fibo(const size_t hash, const size_t shift)
{
	const size_t xor_hash = hash ^ (hash >> shift);
	return (HFIBO * xor_hash) >> shift;
}
//----------------------------------------------------------------- Function ---
static inline size_t hash_fixed128(const char *key)
{
	size_t k1 = *(uint64_t *)key;
	size_t k2 = *(uint64_t *)(key + 8);
	return k1 ^ k2;
}
//----------------------------------------------------------------- Function ---
/**
 * Return a multiplicative style hash for given key.
 *
 * see Linear congruential generator.
 * todo Check what happens with the while loop on compilation.
 */
static inline size_t hash_multiplicative(const char *const key, size_t length)
{
	HFUNC_REGISTER size_t hash = 0;
	// HFUNC_REGISTER size_t i               = length;
	HFUNC_REGISTER const unsigned char *c = (const unsigned char *)key;

	while (length--) {
		hash = HSEED * hash + *c++;
	}

	return hash;
}
//----------------------------------------------------------------- Function ---
/**
 * Return a 64 bits fnv-1a style hash for given key.
 */
static inline size_t hash_fnv1a(const char *const key, size_t length)
{
	HFUNC_REGISTER size_t hash = 0;
	// HFUNC_REGISTER size_t i               = length;
	HFUNC_REGISTER const unsigned char *c = (const unsigned char *)key;

	while (length--) {
		hash ^= *c++;
		hash += (hash << 1) + (hash << 4) + (hash << 5) + (hash << 7) +
		        (hash << 8) + (hash << 40);
	}

	return hash;
}
//----------------------------------------------------------------- Function ---
/**
 * Return a perl style hash for given key.
 *
 * todo Check what happens with the while loop on compilation.
 */
static inline size_t hash_perl(const char *const key, size_t length)
{
	HFUNC_REGISTER size_t hash = HSEED;
	// HFUNC_REGISTER size_t i               = HMAP_INLINE_KEY_SIZE;
	HFUNC_REGISTER const unsigned char *c = (const unsigned char *)key;

	while (length--) {
		hash += *c++;
		hash += hash << 10;
		hash ^= hash >> 6;
	}
	hash += hash << 3;
	hash ^= hash >> 11;
	hash += hash << 15;

	return hash;
}
//----------------------------------------------------------------- Function ---
/**
 * Return a djb2 style hash for given key.
 */
static inline size_t hash_djb2(const char *const key, size_t length)
{
	HFUNC_REGISTER size_t hash = 5381;
	// HFUNC_REGISTER size_t i               = HMAP_INLINE_KEY_SIZE;
	HFUNC_REGISTER const unsigned char *c = (const unsigned char *)key;

	while (length--) {
		hash += (hash << 5) + *c++;
	}

	return hash;
}
//----------------------------------------------------------------- Function ---
/**
 * Return a djb2_alt style hash for given key.
 */
static inline size_t hash_djb2_alt(const char *const key, size_t length)
{
	HFUNC_REGISTER size_t hash = 5381;
	// HFUNC_REGISTER size_t i               = HMAP_INLINE_KEY_SIZE;
	HFUNC_REGISTER const unsigned char *c = (const unsigned char *)key;

	while (length--) {
		hash *= 33 ^ *c++;
	}

	return hash;
}

//----------------------------------------------------------------- Function ---
/**
 * Return a kh_hash_str style hash for given key.
 */
static inline size_t hash_kh_str(const char *const key, size_t length)
{
	HFUNC_REGISTER size_t hash = HSEED;
	// HFUNC_REGISTER size_t i               = HMAP_INLINE_KEY_SIZE;
	HFUNC_REGISTER const unsigned char *c = (const unsigned char *)key;

	while (length--) {
		hash = (hash << 5) - hash + *c++;
	}

	return hash;
}

/*
  Author: Wang Yi <godspeed_china@yeah.net>
  This is a quick and dirty hash function designed for O(1) speed.
  It makes your hash table application fly in most cases.
  It samples first, middle and last 4 bytes to produce the hash.
  Do not use it in very serious applications as it's not secure.
*/
#ifndef O1HASH_LITTLE_ENDIAN
#if defined(_WIN32) || defined(__LITTLE_ENDIAN__) ||                           \
    (defined(__BYTE_ORDER__) && __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__)
#define O1HASH_LITTLE_ENDIAN 1
#elif defined(__BIG_ENDIAN__) ||                                               \
    (defined(__BYTE_ORDER__) && __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__)
#define O1HASH_LITTLE_ENDIAN 0
#endif
#endif
#if (O1HASH_LITTLE_ENDIAN)
static inline unsigned _o1r4(const uint8_t *p)
{
	unsigned v;
	memcpy(&v, p, 4);
	return v;
}
#else
#if defined(__GNUC__) || defined(__INTEL_COMPILER) || defined(__clang__)
static inline unsigned _o1r4(const uint8_t *p)
{
	unsigned v;
	memcpy(&v, p, 4);
	return __builtin_bswap32(v);
}
#elif defined(_MSC_VER)
static inline unsigned _o1r4(const uint8_t *p)
{
	unsigned v;
	memcpy(&v, p, 4);
	return _byteswap_ulong(v);
}
#endif
#endif
//----------------------------------------------------------------- Function ---
static inline uint64_t o1hash(const void *key, size_t len)
{
	const uint8_t *p = (const uint8_t *)key;
	if (len >= 4) {
		unsigned first = _o1r4(p), middle = _o1r4(p + (len >> 1) - 2),
		         last = _o1r4(p + len - 4);
		return (uint64_t)(first + last) * middle;
	}
	if (len) {
		uint64_t tail = ((((unsigned)p[0]) << 16) |
		                 (((unsigned)p[len >> 1]) << 8) | p[len - 1]);
		return tail * 0xa0761d6478bd642full;
	}
	return 0;
}

#endif /* HFUNC_H */