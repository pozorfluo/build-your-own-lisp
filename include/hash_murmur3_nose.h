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
#ifndef HASH_MURMUR3_NOSE_H
#define HASH_MURMUR3_NOSE_H

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#include <stddef.h> /* size_t */
#include <stdint.h> /* uint32_t, uint64_t */

//---------------------------------------------------------- PLATFORM MACROS ---
#ifndef __GNUC__
#define __attribute__(x) /* Nothing */
#endif

//------------------------------------------------------------- DECLARATIONS ---
typedef struct Hash128 {
	uint64_t hi;
	uint64_t lo;
} Hash128;

uint32_t murmurhash3_x86_32(const void *key,
                            const size_t len,
                            const uint32_t seed) __attribute__ ((pure));

Hash128 murmurhash3_x86_128(const void *key,
                            const size_t len,
                            const uint32_t seed) __attribute__ ((pure));

Hash128 murmurhash3_x64(const void *key, const size_t len, const uint32_t seed)
   __attribute__ ((pure));

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // HASH_MURMUR3_NOSE_H