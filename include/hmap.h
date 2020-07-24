//------------------------------------------------------------------ SUMMARY ---
/**
 * Implement a hash map tuned for lispy / LispEnv
 */

#ifndef HMAP_H
#define HMAP_H

#include <stddef.h> /* size_t */
#include <stdint.h> /* SIZE_MAX */
//------------------------------------------------------------ CONFIGURATION ---
// #define SIMD_PROBE
#if SIZE_MAX == 0xffffffffffffffffllu
#define HWIDTH_64
#define HWIDTH 64
#else
#define HWIDTH 32
#endif
#define HMAP_INLINE_KEY_SIZE 16
#define HMAP_MAX_LOAD 0.9 //0.75
#define HMAP_STORE_GROW 1.25
#define HFUNC hash_fnv1a
#define HREDUCE reduce_fibo
#define HCMP strncmp
// #define HCOPY strcpy

//------------------------------------------------------------ MAGIC NUMBERS ---
#define HMAP_NOT_FOUND SIZE_MAX
#define HMAP_PROBE_LENGTH 32
//------------------------------------------------------------------- MACROS ---

//------------------------------------------------------------- DECLARATIONS ---
// typedef signed char meta_byte;
typedef signed short meta_byte;

// enum meta_ctrl {
// 	META_EMPTY    = -128, /* 0b10000000 */
// 	META_OCCUPIED = 0,    /* 0b0xxxxxxx */
// 	/* Think of META_OCCUPIED as anything like 0b0xxxxxxx */
// 	/* aka 0 <= META_OCCUPIED < 128 */
// };
enum meta_ctrl {
	META_EMPTY    = -32768,
	META_OCCUPIED = 0,
};

/**
 * note
 *   Enums are int, use #define MY_OPTION_1 0x01u for bit flags.
 */
// enum value_type {
// 	//----------------------------------------------- inline
// 	V_UINT,
// 	V_2_UINT,
// 	V_4_UINT,
// 	V_8_UINT,
// 	V_INT,
// 	V_2_INT,
// 	V_4_INT,
// 	V_8_INT,
// 	V_2_FLOAT,
// 	V_DOUBLE,
// 	V_ISTR,
// 	//--------------------------------------------- pointers
// 	V_FUNC, /* Do not manage pointee */
// 	V_PTR,  /* Do not manage pointee */
// 	V_OBJ,  /* Will manage pointee */
// 	V_STR   /* Will manage pointee */
// };
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
	meta_byte meta; /* 2 bytes */
	meta_byte distance; /* 2 bytes */
	uint32_t entry; /* can address 2^32 entries */
	/**
	 * uint32_t
     *   (* (expt 2 32) (+ 8 16 8))
     *                   4,294,967,296 entries
     *   >             137,438,953,472 bytes for an hypothetical fully filled
	 *     table with this config.
	 * 
     * uint64_t
     *   (* (expt 2 64) (+ 16 16 8))
     *      18,446,744,073,709,551,616 entries
     *   > 737,869,762,948,382,064,640 bytes for an hypothetical fully filled
	 *     table with this config.
	 */
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
	size_t store_capacity; /* store capacity */
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

struct hmap *debug_grow(struct hmap *const hm);
void debug_rehash(struct hmap *const hm);

#endif /* HMAP_H */