//------------------------------------------------------------------ SUMMARY ---
/**
 * Implement a hash map tuned for lispy / LispEnv
 */

#ifndef HMAP_H
#define HMAP_H

#include <stddef.h> /* size_t */
//------------------------------------------------------------ CONFIGURATION ---
// #define SIMD_PROBE
#define HMAP_INLINE_KEY_SIZE 16
#define HFUNC hash_multiplicative
#define HREDUCE reduce_fibo
#define HCMP strncmp
#define HCOPY strcpy
//------------------------------------------------------------ MAGIC NUMBERS ---
#define HMAP_NOT_FOUND SIZE_MAX
#define HMAP_PROBE_LENGTH 32
//------------------------------------------------------------------- MACROS ---

//------------------------------------------------------------- DECLARATIONS ---
typedef signed char meta_byte;

enum meta_ctrl {
	META_EMPTY    = -128, /* 0b10000000 */
	META_OCCUPIED = 0,    /* 0b0xxxxxxx */
	/* Think of META_OCCUPIED as anything like 0b0xxxxxxx */
	/* aka 0 <= META_OCCUPIED < 128 */
};

/**
 * todo Rework this for a more generic setup when you are more comfortable using
 *      unions.
 */
struct hmap_entry {
	// size_t key;
	// char *key; /* string key stored elsewhere */
	char key[HMAP_INLINE_KEY_SIZE]; /* string key stored inline are not null
	                                   terminated */
	size_t value;
};

struct hmap_bucket {
	meta_byte meta;     /* 1 byte per bucket */
	meta_byte distance; /* 1 byte per bucket */
	size_t entry;       /* sizeof(size_t) bytes per bucket */
};

/**
 * Check packing https://godbolt.org/z/dvdKqM
 */
struct hmap {
	struct hmap_bucket *buckets;
	struct hmap_entry *store;
	size_t top; /* occupied entries count, cursor to next free store slot */
	// size_t key_size;
	size_t hash_shift; /* shift amount necessary for desired hash depth */
	size_t capacity;   /* actual capacity */
};

size_t hmap_find(const struct hmap *const hashmap, const char *const key)
    __attribute__((pure));

size_t hmap_get(const struct hmap *const hm, const char *const key);

size_t hmap_put(struct hmap *const hm,
                const char *const key,
                const size_t value);

size_t hmap_remove(struct hmap *const hm, const char *const key);

struct hmap *hmap_new(const size_t n);

void hmap_clear(struct hmap *const hm);

void hmap_free(struct hmap *const hashmap);

#endif /* HMAP_H */