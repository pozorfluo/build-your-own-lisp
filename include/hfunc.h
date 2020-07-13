//------------------------------------------------------------------ SUMMARY ---
/**
 * Collection of hash functions and hash reductions.
 *
 * todo Consider testing Lemire's fast range.
 *        See https://lemire.me/blog/2016/06/27/a-fast-alternative-to-the-modulo-reduction/
 *        See https://github.com/lemire/fastrange/blob/master/fastrange.h
 */

//-------------------------------------------------------------------- HFUNC ---
#ifndef HFUNC_H
#define HFUNC_H

#include <stddef.h>   /* size_t */

//------------------------------------------------------------ CONFIGURATION ---
#include "configuration.h"

#ifdef TEST_REGISTER
#define HFUNC_REGISTER register
#else
#define HFUNC_REGISTER
#endif /* TEST_REGISTER */
//------------------------------------------------------------ MAGIC NUMBERS ---
#define HFIBO 11400714819323198485llu
#define HSEED 11400714819323198485llu
//------------------------------------------------------------- DECLARATIONS ---
static inline size_t hash_perl(const char *const key)
    __attribute__((pure, always_inline));

static inline size_t hash_multiplicative(const char *const key)
    __attribute__((pure, always_inline));

static inline size_t hash_djb2(const char *const key)
    __attribute__((pure, always_inline));

static inline size_t hash_djb2_alt(const char *const key)
    __attribute__((pure, always_inline));

static inline size_t hash_kh_str(const char *const key)
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
/**
 * Return a multiplicative style hash for given key.
 *
 * see Linear congruential generator.
 * todo Check what happens with the while loop on compilation.
 */
static inline size_t hash_multiplicative(const char *key)
{
	HFUNC_REGISTER size_t hash            = 0;
	HFUNC_REGISTER size_t i               = HMAP_INLINE_KEY_SIZE;
	HFUNC_REGISTER const unsigned char *c = (const unsigned char *)key;

	while (i--) {
		hash = HSEED * hash + *c++;
	}

	return hash;
}
//----------------------------------------------------------------- Function ---
/**
 * Return a perl style hash for given key.
 *
 * todo Check what happens with the while loop on compilation.
 */
static inline size_t hash_perl(const char *key)
{
	HFUNC_REGISTER size_t hash            = HSEED;
	HFUNC_REGISTER size_t i               = HMAP_INLINE_KEY_SIZE;
	HFUNC_REGISTER const unsigned char *c = (const unsigned char *)key;

	while (i--) {
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
static inline size_t hash_djb2(const char *key)
{
	HFUNC_REGISTER size_t hash            = 5381;
	HFUNC_REGISTER size_t i               = HMAP_INLINE_KEY_SIZE;
	HFUNC_REGISTER const unsigned char *c = (const unsigned char *)key;

	while (i--) {
		hash += (hash << 5) + *c++;
	}

	return hash;
}
//----------------------------------------------------------------- Function ---
/**
 * Return a djb2_alt style hash for given key.
 */
static inline size_t hash_djb2_alt(const char *key)
{
	HFUNC_REGISTER size_t hash            = 5381;
	HFUNC_REGISTER size_t i               = HMAP_INLINE_KEY_SIZE;
	HFUNC_REGISTER const unsigned char *c = (const unsigned char *)key;

	while (i--) {
		hash *= 33 ^ *c++;
	}

	return hash;
}

//----------------------------------------------------------------- Function ---
/**
 * Return a kh_hash_str style hash for given key.
 */
static inline size_t hash_kh_str(const char *key)
{
	HFUNC_REGISTER size_t hash            = HSEED;
	HFUNC_REGISTER size_t i               = HMAP_INLINE_KEY_SIZE;
	HFUNC_REGISTER const unsigned char *c = (const unsigned char *)key;

	while (i--) {
		hash = (hash << 5)- hash + *c++;
	}

	return hash;
}

#endif /* HFUNC_H */