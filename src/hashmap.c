//------------------------------------------------------------------ SUMMARY ---
/**
 * Based on the work and publications of the Abseil Team, Daniel Lemire,
 * Peter Kankowski.
 *
 * Implement a hash map tuned for lispy / LispEnv
 * 
 *
 * todo
 *   - [ ] Use fast range instead of modulo if tab_hash isn't fit, bitmask if
 *         desperate
 */ 
 
#include <stddef.h> /* size_t */
#include <stdint.h> /* uint32_t, uint64_t */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* __rdtsc() _mm_set1_epi8 _mm_movemask_epi8 _mm_cmpeq_epi8 */
#ifdef _MSC_VER
#include <intrin.h>
#else
// #include <immintrin.h>
#include <x86intrin.h>
#endif

#include "ansi_esc.h"

#include <assert.h>
// #define DEBUG_HMAP

#include "debug_xmalloc.h"

#ifdef DEBUG_HMAP
#include <math.h> /* pow() */
#define DEBUG_MALLOC
#endif /* DEBUG_HMAP */
//------------------------------------------------------------ MAGIC NUMBERS ---
#define PROBE_LIMIT 16

//------------------------------------------------------------------- MACROS ---
#define ARRAY_LENGTH(_array) (sizeof(_array) / sizeof((_array)[0]))

//------------------------------------------------------------- DECLARATIONS ---
typedef signed char meta_byte;
/**
 * | bit 7 | bit 6 | bit 5 | bit 4 | bit 3 | bit 2 | bit 1 | bit 0 |
 * |-------|-------|-------|-------|-------|-------|-------|-------|
 * |   0   |  hash |  hash |  hash |  hash |  hash |  hash |  hash |
 * |   1   | state | state | state | state | state | state | state |
 *
 *    MSB is 0 -> entry is OCCUPIED,     use 7 bits remaining as hash
 *    MSB is 1 -> entry is NOT OCCUPIED, use 7 bits remaining as state
 */
enum meta_ctrl {
	META_EMPTY   = -128, /* 0b10000000 */
	META_DELETED = -2,   /* 0b11111110 */
	META_MARKED  = -1,   /* 0b11111111 */
	/* Think of META_OCCUPIED as anything like 0b0xxxxxxx */ 
	/* aka 0 <= META_OCCUPIED < 128 */
};

struct hmap_allocator {
	void *(*malloc)(size_t size);
	void (*free)(void *);
};

#ifdef DEBUG_HMAP
struct hmap_stats {
	size_t hashes_tally_or;
	size_t hashes_tally_and;
	size_t put_count;
	size_t collision_count;
	size_t hashes_ctrl_collision_count;
	size_t putfail_count;
	size_t del_count;
	size_t find_count;
	unsigned int n;
};
#endif /* DEBUG_HMAP */

struct hmap_buckets {
	meta_byte *metas;
	char **keys;
	void **values;
};

/**
 * Check packing if necessary : https://godbolt.org/z/295NFB
 */
struct hmap {
	const size_t xor_seed[256];
	struct hmap_buckets buckets;
	const struct hmap_allocator allocator;
	size_t actual_capacity; /* advertised capacity + PROBE_LIMIT */
	size_t capacity;        /* advertised capacity               */
	size_t count;           /* occupied entries                  */
#ifdef DEBUG_HMAP
	struct hmap_stats stats;
#endif /* DEBUG_HMAP */
};

struct hmap_init {
	size_t xor_seed[256];
	struct hmap_buckets buckets;
	struct hmap_allocator allocator;
	int probe_limit;
	size_t capacity;
	size_t count;
#ifdef DEBUG_HMAP
	struct hmap_stats stats;
#endif /* DEBUG_HMAP */
};

//----------------------------------------------------- FORWARD DECLARATIONS ---
static inline size_t hash_tab(const unsigned char *key, const size_t *xor_seed)
    __attribute__((const, always_inline));

static inline size_t hash_index(const size_t hash)
    __attribute__((const, always_inline));

static inline meta_byte hash_meta(const size_t hash)
    __attribute__((const, always_inline));

static inline int compare_keys(const char *const key_a, const char *const key_b)
    __attribute__((const, always_inline));

static inline uint16_t probe_chunk(const meta_byte pattern,
                                   const meta_byte *entry)
    __attribute__((const, always_inline));

// static inline size_t hmap_find(const struct hmap *hashmap, const char *key)
//     __attribute__((const, always_inline));

static inline void xor_seed_fill(size_t *xor_seed, size_t hash_depth)
    __attribute__((always_inline));

static inline void destroy_entry(struct hmap *hashmap, size_t entry)
    __attribute__((always_inline));

//----------------------------------------------------------------- Function ---
/**
 * Compute a tabulation style hash for given key, xor_seed table
 *   -> hmap index
 *
 * todo
 *   - [ ] Assess hash function fitness
 *     + [x] Track stuck bits with hashes_tally_or, hashes_tally_and
       + [ ] Track control chunk collisions likelyhood with
             hashes_ctrl_collision_count
       + [ ] Look at SMHasher for more ways to assess fitness
         * [ ] See : https://github.com/aappleby/smhasher/wiki/SMHasher
 */
static inline size_t hash_tab(const unsigned char *key, const size_t *xor_seed)
{
	size_t hash = 0;

	while (*key) {
		hash ^= xor_seed[(*key + hash) & 0xFF] ^ *key;
#ifdef DEBUG_HMAP
		printf(
		    "partial hash : %lu\n"
		    "char         : %c\n"
		    "xor[char]    : %lu\n\n",
		    hash,
		    (char)*key,
		    xor_seed[*key]);
#endif /* DEBUG_HMAP */
		key++;
	}

	return hash;
}

static inline size_t hash_index(const size_t hash) { return hash >> 7; }

static inline meta_byte hash_meta(const size_t hash) { return hash & 0x7F; }

//----------------------------------------------------------------- Function ---
/**
 * Check if given entry in given hashmap is empty
 *   -> Truth value of predicate
 */
static inline int is_empty(const meta_byte meta)
{
	return (meta == META_EMPTY);
}
static inline int is_entry_empty(struct hmap *hashmap, size_t entry)
{
	return is_empty(hashmap->buckets.metas[entry]);
}

static inline int is_full(const meta_byte meta) { return (meta == META_EMPTY); }

//----------------------------------------------------------------- Function ---
/**
 * Create a new entry in given Hashmap for given key, value pair
 *
 *   Get hash for given key
 *   Locate bucket for that hash
 *   If bucket is empty
 * 	   Create new entry
 *     Insert entry in bucket
 *     Increment Hashmap entry count ( Check Hashmap fitness )
 * 		-> nothing
 *   Else !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
 *     If key found
 *       Delete value
 *       Replace with copy of given value
 * !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
 *     -> nothing
 */
size_t put_hashmap(struct hmap *hashmap, const char *key, void *value)
{
	assert(key != NULL);
	assert(value != NULL);

	size_t hash = hash_tab((unsigned char *)key, hashmap->xor_seed);
	hashmap->stats.hashes_tally_or |= hash;
	hashmap->stats.hashes_tally_and &= hash;

	// printf("hash->[%lu]\n", hash);
	// HashmapEntry *bucket = &hashmap->buckets[hash];
	/**
	 * todo
	 *   - [ ] Check if this compile to something different
	 */
	// unsigned char *meta;
	// int * distance;
	// char **key;
	// void **value;

	unsigned char entry_meta = hash_meta(hash);
	size_t entry_index       = hash_index(hash);

	__m128i filter = _mm_set1_epi8(entry_meta);
	int match_mask = _mm_movemask_epi8(_mm_cmpeq_epi8(filter), );

	/* Prepare temp entry*/
	struct hmap_buckets entry;
	entry.distance = 0;
	entry.key      = XMALLOC(strlen(key) + 1, "put_hashmap", "temp_entry.key");
	strcpy(entry.key, key);
	entry.value = value;

	for (; entry.distance < hashmap->probe_limit; entry.distance++, bucket++) {
		/* Empty bucket */
		if (bucket->key == NULL) {
			bucket->key      = entry.key;
			bucket->value    = entry.value;
			bucket->distance = entry.distance;
			// check_hashmap(hashmap);
			hashmap->count++;

			/* Solved collision */
			if (entry.distance > 0) {
				hashmap->stats.collision_count++;
			}
			hashmap->stats.put_count++;
			return bucket;
		}
		else {
			/* Key exist */
			// todo
			//   - [ ] Probe for existing key
			//   - [ ] Make sure comparison is necessary and cheap
			if (strcmp(entry.key, bucket->key) == 0) {
				XFREE(entry.key, "duplicate key");
				hashmap->destructor(
				    bucket->value); //, "bucket->value", "destructor");
				bucket->value = entry.value;
				hashmap->stats.put_count++;
				return bucket;
			}

			// /* Rich bucket */
			// if (entry.distance > bucket->distance) {
			// 	swap_entries(&entry, bucket);
			// 	hashmap->stats.swap_count++;
			// }
		}
	}

	/* Past probe limit, trigger resize */
	XFREE(entry.key, "past probe limit");
	hashmap->stats.putfail_count++;
	return NULL;
}

//----------------------------------------------------------------- Function ---
/**
 * Compare given '\0' terminated string keys
 *   If length of given keys does NOT match
 *     -> -1
 *   Else
 *     If bytes comparison matches over length + 1
 *       -> 0
 *     Else
 *       -> difference
 */
static inline int compare_keys(const char *const key_a, const char *const key_b)
{
	size_t size_a  = strlen(key_a);
	size_t size_b  = strlen(key_b);
	int difference = -1;

	// do not bother with switch & jump, gcc outputs the same assembly
	// see : https://godbolt.org/z/suQQMk
	if (size_a == size_b) {
		difference = memcmp(key_a, key_b, size_a + 1);
	}
	return difference;
}

#ifdef DEBUG_HMAP
//----------------------------------------------------------------- Function ---
/**
 * Pretty print bits for given n bytes at given data pointer
 *   Handle little endian only
 *   -> nothing
 */
void print_bits(const size_t n, void const *const data)
{
	unsigned char *bit = (unsigned char *)data;
	unsigned char byte;

	for (int i = n - 1; i >= 0; i--) {
		for (int j = 7; j >= 0; j--) {
			byte = (bit[i] >> j) & 1;
			printf("%u", byte);
		}
	}

	putchar('\n');
}

void print_m128i(const __m128i value)
{
	printf("%016llx%016llx\n", value[1], value[0]);
}

void print_m128i_hexu8(const __m128i value)
{
	// _Alignas(uint16_t) uint16_t vec[8];
	uint8_t vec[16];
	_mm_store_si128((__m128i *)vec, value);
	printf(
	    "%02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x "
	    "%02x %02x\n",
	    vec[0],
	    vec[1],
	    vec[2],
	    vec[3],
	    vec[4],
	    vec[5],
	    vec[6],
	    vec[7],
	    vec[8],
	    vec[9],
	    vec[10],
	    vec[11],
	    vec[12],
	    vec[13],
	    vec[14],
	    vec[15]);
}
#endif /* DEBUG_HMAP */
//----------------------------------------------------------------- Function ---
/**
 * Probe metadata chunk of 16 bytes for given ctrl_byte pattern starting at
 * given metadata entry
 *   -> Matches bitmask
 */
static inline uint16_t probe_chunk(const meta_byte pattern,
                                   const meta_byte *entry)
{
	/* setup filter */
	const __m128i filter = _mm_set1_epi8(pattern);

	/* filter chunks */
	const __m128i chunk = _mm_loadu_si128((__m128i *)(entry));
	const __m128i match = _mm_cmpeq_epi8(filter, chunk);

#ifdef DEBUG_HMAP
	puts("filter :");
	print_m128i_hexu8(filter);
	print_m128i_hexu8(chunk);
#endif /* DEBUG_HMAP */

	return _mm_movemask_epi8(match);
}

//----------------------------------------------------------------- Function ---
/**
 * Locate given key in given hmap
 * If given key exists
 *   -> entry index
 * Else
 *   -> out of bound value ( > actual_capacity)
 * 
 * Current implementation :
 *   Probe
 */
size_t hmap_find(const struct hmap *hashmap, const char *key)
{
	size_t hash    = hash_tab((unsigned char *)key, hashmap->xor_seed);
	size_t index   = hash_index(hash);
	meta_byte meta = hash_meta(hash);
	size_t entry   = hashmap->actual_capacity + 1;

	uint16_t match_mask = probe_chunk(meta, hashmap->buckets.metas + index);
	print_bits(2, &match_mask);

	/* loop through set bit in bitmask to access matches */
	while (match_mask != 0) {
		const size_t offset    = __builtin_ctz(match_mask);
		const char *probed_key = hashmap->buckets.keys[index + offset];

		if ((probed_key != NULL) && (compare_keys(probed_key, key)) == 0) {
			// hashmap->stats.find_count++;
			entry = index + offset;
		}
		// else {
		// hashmap->stats.hashes_ctrl_collision_count++;
		// }
		/* remove least significant set bit */
		match_mask ^= match_mask & (-match_mask);
	}

	return entry;
}

// size_t hmap_find(const struct hmap *hashmap, const char *key)
// {
// 	size_t hash  = hash_tab((unsigned char *)key, hashmap->xor_seed);
// 	size_t index = hash_index(hash);
// 	// ctrl_byte meta = hash_meta(hash);

// 	uint16_t match_mask =
// 	    probe_chunk(CTRL_EMPTY, hashmap->buckets.metas + index);
// 	print_bits(2, &match_mask);

// 	/* loop through set bit in bitmask to access matches */
// 	while (match_mask != 0) {
// 		const size_t offset = __builtin_ctz(match_mask);

// #ifdef DEBUG_HMAP
// 		printf("%s vs %s\n", hashmap->buckets.keys[index + offset], key);
// #endif /* DEBUG_HMAP */

// 		/**
// 		 * todo
// 		 *   - [ ] Make sure hmap_find is never called to find empty slots or
// 		 *     + [ ] Have a separate hmap_find_empty
// 		 *   - [ ] Assert that matched entry key is NOT NULL before comparing
// 		 */
// 		const char *probed_key = hashmap->buckets.keys[index + offset];

// 		if ((probed_key != NULL) && (compare_keys(probed_key, key)) == 0) {
// #ifdef DEBUG_HMAP
// 			printf("                          -> found %s @ %lu + %lu \n",
// 			       hashmap->buckets.keys[index + offset],
// 			       index,
// 			       offset);
// #endif      /* DEBUG_HMAP */
// 			// return index + offset;
// 		}
// 		else {
// #ifdef DEBUG_HMAP
// 			printf("                          -> %s collision @ %lu + %lu \n",
// 			       hashmap->buckets.keys[index + offset],
// 			       index,
// 			       offset);
// #endif /* DEBUG_HMAP */
// 		}
// #ifdef DEBUG_HMAP
// 		printf("offset                    : %lu\n", offset);
// 		printf("match_mask                : %d 0b", match_mask);
// 		print_bits(2, &match_mask);
// 		putchar('\n');
// #endif /* DEBUG_HMAP */

// 		/* remove least significant set bit */
// 		match_mask ^= match_mask & (-match_mask);
// 	}
// 	/**
// 	 * todo
// 	 *   - [ ] fix silly return value
// 	 */
// 	return NULL;
// }
//----------------------------------------------------------------- Function ---
/**
 * Locate given value for given key in given hmap
 *   -> pointer to value in hmap
 */
// void *hmap_get(const struct hmap *hashmap, const char *key) {}

//----------------------------------------------------------------- Function ---
/**
 * Free given HashmapEntry
 *   -> nothing
 *
 * note
 *   Require user to allocate for given value to be inserted in the hashmap
 *   Require user to provide destructor for given value
 *   Free value with given destructor
 */
static inline void destroy_entry(struct hmap *hashmap, size_t entry)
{
	// Use XFREE macro instead of destructor while hmap is a WIP
	XFREE(hashmap->buckets.keys[entry], hashmap->buckets.keys[entry]);
	// destructor(hashmap->buckets.keys[entry]);
	XFREE(hashmap->buckets.values[entry], "destroy_entry");
	return;
}
//----------------------------------------------------------------- Function ---
/**
 * Look for given key in given hmap
 * If given key exists
 *   Destroy entry associated to given key
 *   Mark metadata associated to given key as CTRL_DELETED
 *   Update given hmap stats
 *   -> destroyed entry index
 * Else
 *   -> out of bound value ( > actual_capacity )
 */
size_t delete_hashmap_entry(struct hmap *hashmap, const char *key)
{
	size_t entry = hmap_find(hashmap, key);

	if (entry < hashmap->actual_capacity) {
		destroy_entry(hashmap, entry);
		hashmap->buckets.metas[entry] = META_DELETED; //|=META_DELETED; useful ?
		hashmap->count--;
	}

#ifdef DEBUG_HMAP
	hashmap->stats.del_count++;
#endif /* DEBUG_HMAP */
	return entry;
}

//----------------------------------------------------------------- Function ---
/**
 * Free given Hashmap
 *   -> nothing
 *
 * todo
 *   - [ ] Consider a clear_hashmap func : destroy entries only
 *   - [ ] Consider a reset_hashmap func : destroy entries, reset hashmap to
 *         given new size
 */
void hmap_delete_hashmap(struct hmap *hashmap)
{
	for (size_t entry = 0; entry < hashmap->actual_capacity; entry++) {
		if (hashmap->buckets.keys[entry] == NULL) {
			continue;
		}
		// printf("%s\n", hashmap->buckets[i].key);
		destroy_entry(hashmap, entry);
#ifdef DEBUG_HMAP
		hashmap->stats.del_count++;
#endif /* DEBUG_HMAP */
	}
	// stats_hashmap(hashmap);
	XFREE(hashmap->buckets.metas, "delete_hashmap hashmap->buckets.metas");
	XFREE(hashmap->buckets.keys, "delete_hashmap hashmap->buckets.keys");
	XFREE(hashmap->buckets.values, "delete_hashmap hashmap->buckets.value");
	XFREE(hashmap, "delete_hashmap");
}

//----------------------------------------------------------------- Function ---
/**
 * Fill given xor_seed for given hash depth
 *   -> pointer to new xor_seed
 */
static inline void xor_seed_fill(size_t *xor_seed, size_t hash_depth)
{
	srand(__rdtsc());
	for (size_t i = 0; i < 256; i++) {
		// % capacity is okay-ish as long as its a power of 2
		// Expect skewed distribution otherwise
		xor_seed[i] = rand() % (hash_depth);
	}
}

//----------------------------------------------------------------- Function ---
/**
 * Create a new hmap of capacity equal to 2^(given n),  with given seed
 *
 * 	 Init allocator to custom given allocator or default if none given
 *   Enforce n >= 1
 *   Init a dummy xor_seed with hash_depth set to yield n + 7 ctrl bits
 *   Init a dummy hmap_init
 * 	 Copy dummy xor_seed in dummy HashmapInit
 *   Allocate a Hashmap
 *     If allocation fails,  clean up and abort
 *     -> NULL
 *   Copy dummy HashmapInit to Hashmap
 *   Allocate Hashmap buckets
 *     If any allocation fails, clean up and abort
 *     -> NULL
 *   Init Hashmap buckets
 *     -> pointer to new hmap
 *
 * todo
 *   - [ ] Replace parameter n with requested_capacity
 *     + [ ] Compute next 2^(n) - 1 from requested_capacity
 *     + [x] Store n in debug stats
 *   - [ ] Make sure hashmap_init cannot blow up the stack on HUGE sizes
 *     + [ ] Handle such cases if any with branching to an alloc
 *     + [ ] Consider dropping the whole hashmap_init step and the const
 *           qualifier on xor_seed_init as workaround
 *   - [ ] Consider one big alloc of
 *         sizeof(metas +  keys + values ) * actual_capacity
 *     + [ ] Consider setting pointers inside the struct accordingly
 *           after the one big alloc
 */
struct hmap *hmap_new(const unsigned int n,
                      const struct hmap_allocator *const allocator)
{
	//--------------------------------------------------- allocator init
	struct hmap_allocator allocator_init;

	if (allocator == NULL) {
		allocator_init.malloc = malloc;
		allocator_init.free   = free;
	}
	else {
		allocator_init = *allocator;
	}
	//---------------------------------------------- advertised capacity
	/* tab_hash requires a mininum of 8 bits of hash space */
	/* including the 7 bits required for ctrl_byte */
	const size_t capacity   = (n < 1) ? (1u << 1) : (1u << n);
	const size_t hash_depth = capacity << 7;

	//---------------------------------------------------- xor_seed init
	size_t xor_seed_init[256];
	xor_seed_fill(xor_seed_init, hash_depth);

	//-------------------------------------------------------- hmap init
	const size_t actual_capacity = capacity + PROBE_LIMIT;

#ifdef DEBUG_HMAP
	struct hmap_stats stats_init  = {0, SIZE_MAX, 0, 0, 0, 0, 0, 0, n};
	struct hmap_init hashmap_init = {
	    {0}, {NULL}, allocator_init, actual_capacity, capacity, 0, stats_init};
#else
	struct hmap_init hashmap_init = {
	    {0}, {NULL}, allocator_init, actual_capacity, capacity, 0};
#endif /* DEBUG_HMAP */

	memcpy(hashmap_init.xor_seed, xor_seed_init, sizeof(xor_seed_init));

	struct hmap *new_hashmap =
	    XMALLOC(sizeof(struct hmap), "new_hashmap", "new_hashmap");
	memcpy(new_hashmap, &hashmap_init, sizeof(struct hmap));
	if (new_hashmap == NULL) {
		goto err_free_hashmap;
	}

	new_hashmap->buckets.metas =
	    XMALLOC(sizeof(*(new_hashmap->buckets.metas)) * actual_capacity,
	            "new_hashmap",
	            "new_hashmap->buckets.metas");
	if (new_hashmap->buckets.metas == NULL) {
		goto err_free_buckets;
	}
	new_hashmap->buckets.keys =
	    XMALLOC(sizeof(*(new_hashmap->buckets.keys)) * actual_capacity,
	            "new_hashmap",
	            "new_hashmap->buckets.keys");
	if (new_hashmap->buckets.keys == NULL) {
		goto err_free_buckets;
	}
	new_hashmap->buckets.values =
	    XMALLOC(sizeof(*(new_hashmap->buckets.values)) * actual_capacity,
	            "new_hashmap",
	            "new_hashmap->buckets.values");
	if (new_hashmap->buckets.values == NULL) {
		goto err_free_buckets;
	}

	/* The value is passed as an int, but the function fills the block of
	 * memory using the unsigned char conversion of this value */
	memset(new_hashmap->buckets.metas,
	       META_EMPTY,
	       sizeof(*(new_hashmap->buckets.metas)) * actual_capacity);
	/* Is this enough to be able to check if ptr == NULL ? */
	memset(new_hashmap->buckets.keys,
	       0,
	       sizeof(*(new_hashmap->buckets.keys)) * actual_capacity);
	memset(new_hashmap->buckets.values,
	       0,
	       sizeof(*(new_hashmap->buckets.values)) * actual_capacity);

	return new_hashmap;

/* free(NULL) is ok, correct ? */
err_free_buckets:
	XFREE(new_hashmap->buckets.metas, "new_hashmap->buckets.metas");
	XFREE(new_hashmap->buckets.keys, "new_hashmap->buckets.keys");
	XFREE(new_hashmap->buckets.values, "new_hashmap->buckets.values");
err_free_hashmap:
	XFREE(new_hashmap, "new_hashmap");
	return NULL;
}

#ifdef DEBUG_HMAP
//----------------------------------------------------------------- Function ---
/**
 * Compute the expected number of entries per bucket for given number of
 * keys,
 * buckets
 *   -> expected number of entries per bucket
 */
double expected_entries_per_bucket(size_t keys, size_t buckets)
{
	return (double)keys / (double)buckets;
}
//----------------------------------------------------------------- Function ---
/**
 * Compute the expected number of empty buckets for given number of keys,
 * buckets
 *   -> expected number of empty buckets
 */
double expected_empty_buckets(size_t keys, size_t buckets)
{
	return (double)buckets * pow(1 - (1 / (double)buckets), (double)keys);
}
//----------------------------------------------------------------- Function ---
/**
 * Compute the expected number of filled buckets for given number of keys,
 * buckets
 *   -> expected number of filled buckets
 */
double expected_filled_buckets(size_t keys, size_t buckets)
{
	return (double)buckets - expected_empty_buckets(keys, buckets);
}
//----------------------------------------------------------------- Function ---
/**
 * Compute the expected number of collisions for given number of keys,
 * buckets
 *   -> expected number of collisions
 */
double expected_collisions(size_t keys, size_t buckets)
{
	return (double)keys - expected_filled_buckets(keys, buckets);
}
//----------------------------------------------------------------- Function ---
/**
 * Pretty print xor_seed for given Hashmap
 *   -> nothing
 */
void print_xor_seed(const struct hmap *hashmap)
{
	puts("xor_seed            :");
	for (size_t i = 0; i < 256; i++) {
		printf("%0*lx ",
		       (int)ceil(((hashmap->stats.n + 7) / 4.0)),
		       hashmap->xor_seed[i]);
		if (((i + 1) % 16) == 0) {
			putchar('\n');
		}
	}
	putchar('\n');

	return;
}

//----------------------------------------------------------------- Function ---
/**
 * Print given Hashmap stats
 *   -> nothing
 */
void stats_hashmap(const struct hmap *hashmap)
{
	printf("EV entries / bucket = %f\n",
	       expected_entries_per_bucket(hashmap->count, hashmap->capacity));
	printf("EV collisions = %f \t-> %f%%\n",
	       expected_collisions(hashmap->count, hashmap->capacity),
	       expected_collisions(hashmap->count, hashmap->capacity) /
	           (double)hashmap->count * 100);
	printf("EV empty = %f \t-> %f%%\n",
	       expected_empty_buckets(hashmap->count, hashmap->capacity),
	       expected_empty_buckets(hashmap->count, hashmap->capacity) /
	           (double)hashmap->capacity * 100);

	printf("hashmap->stats.n          : %u\n", hashmap->stats.n);
	printf("hashmap->capacity   : %lu / %lu\n",
	       hashmap->capacity,
	       hashmap->actual_capacity);
	printf("hashmap->count      : %lu \t-> %f%%\n",
	       hashmap->count,
	       (double)hashmap->count / (double)hashmap->capacity * 100);
	printf("hashmap->stats.collision_count  : %lu \t-> %lf%%\n",
	       hashmap->stats.collision_count,
	       (double)hashmap->stats.collision_count / (double)hashmap->count *
	           100);
	printf("hashmap->stats.hashes_ctrl_collision_count  : %lu\n",
	       hashmap->stats.collision_count);

	printf("hashmap->stats.hashes_tally_or  : %lu\n",
	       hashmap->stats.hashes_tally_or);
	printf("hashmap->stats.hashes_tally_and : %lu\n",
	       hashmap->stats.hashes_tally_and);
	printf("hashmap->stats.put_count        : %lu\n", hashmap->stats.put_count);
	printf("hashmap->stats.putfail_count    : %lu\n",
	       hashmap->stats.putfail_count);
	printf("hashmap->stats.del_count        : %lu\n", hashmap->stats.del_count);
	printf("hashmap->stats.find_count       : %lu\n",
	       hashmap->stats.find_count);
	printf("hashmap->stats.collision_count  : %lu\n",
	       hashmap->stats.collision_count);

	return;
}
#endif /* DEBUG_HMAP */
//----------------------------------------------------------------- Function ---
/**
 * Dump given Hashmap content
 *   -> nothing
 *
 * todo
 *   - [ ] Implement list iterator
 *     + [ ] See http://rosettacode.org/wiki/Doubly-linked_list/Traversal#C
 */
void dump_hashmap(const struct hmap *hashmap)
{
	size_t empty_bucket = 0;

	for (size_t i = 0; i < hashmap->actual_capacity; i++) {
		if (hashmap->buckets.keys[i] == NULL) {
			empty_bucket++;
			printf("\x1b[100mhashmap->\x1b[30mbucket[%lu]>> EMPTY <<\x1b[0m\n",
			       i);
			continue;
		}

		printf("\x1b[10%dmhashmap->\x1b[30mbucket[%lu]\x1b[0m>>\n", 4, i);

		// printf("\t %016lx : %s\n",
		printf("\t        %lu : %s\n",
		       hash_tab((unsigned char *)hashmap->buckets.keys[i],
		                hashmap->xor_seed),
		       hashmap->buckets.keys[i]);
	}

	printf("empty_buckets       : %lu \t-> %f%%\n",
	       empty_bucket,
	       (double)empty_bucket / (double)hashmap->capacity * 100);

#ifdef DEBUG_HMAP
	stats_hashmap(hashmap);
#endif /* DEBUG_HMAP */

	print_xor_seed(hashmap);
}

//--------------------------------------------------------------------- MAIN ---
int main(void)
{
	// uint32_t seed = 31;
	size_t n = 8;
	size_t hash;
	// Hashmap *hashmap = new_hashmap(n, seed, hash_fimur_reduce);

	fputs(FG_BRIGHT_BLUE REVERSE
	      "Table size is 2^n. Enter n ( default n=8 ) " FG_BRIGHT_BLUE
	      " ? " RESET,
	      stdout);
	scanf("%lu", &n);

	struct hmap *hashmap = hmap_new(n, NULL);
	//----------------------------------------------------------- mock entry
	hash           = hash_tab((unsigned char *)"mock", hashmap->xor_seed);
	size_t index   = hash_index(hash);
	meta_byte meta = hash_meta(hash);
	hashmap->buckets.metas[index]  = meta;
	hashmap->buckets.keys[index]   = strdup("mock");
	hashmap->buckets.values[index] = strdup("mock value");
	hashmap->count++;

	hashmap->buckets.metas[index + 15]  = meta ^ 0x01;
	hashmap->buckets.keys[index + 15]   = strdup("mokk");
	hashmap->buckets.values[index + 15] = strdup("mokk value");
	hashmap->count++;

	hashmap->buckets.metas[index + 3]  = meta ^ 0x01;
	hashmap->buckets.keys[index + 3]   = strdup("mock");
	hashmap->buckets.values[index + 3] = strdup("mock value");
	hashmap->count++;

	hashmap->buckets.metas[index + 4]  = meta;
	hashmap->buckets.keys[index + 4]   = strdup("mock");
	hashmap->buckets.values[index + 4] = strdup("mock value");
	hashmap->count++;

	hashmap->buckets.metas[index + 11]  = meta;
	hashmap->buckets.keys[index + 11]   = strdup("pock");
	hashmap->buckets.values[index + 11] = strdup("pock value");
	hashmap->count++;

	hashmap->buckets.metas[index + 8]  = meta;
	hashmap->buckets.keys[index + 8]   = strdup("mokk");
	hashmap->buckets.values[index + 8] = strdup("mokk value");
	hashmap->count++;

	hmap_find(hashmap, "mock");

//-------------------------------------------------------------------- setup
#define KEYPOOL_SIZE 32
	// char random_keys[KEYPOOL_SIZE] = {'\0'};
	// size_t test_count              = (1 << (n - 2)) - 1; // 1 << (n - 1);
	char key[256];
	// char *dummy_value = NULL;

	// srand(__rdtsc());

	// for (size_t k = 0; k < test_count; k++) {
	// 	for (size_t i = 0; i < KEYPOOL_SIZE - 1; i++) {
	// 		random_keys[i] = (char)(rand() % 95 + 0x20); // % 26 + 0x61);
	// 		// putchar(random_keys[i]);
	// 	}
	// 	// putchar('\n');

	// 	dummy_value = strdup(&random_keys[rand() % (KEYPOOL_SIZE - 1)]);
	// 	// printf(
	// 	//     "[%lu]key   : %s\n"
	// 	//     "[%lu]value : %s\n",
	// 	//     k,
	// 	//     &random_keys[rand() % (KEYPOOL_SIZE - 1)],
	// 	//     k,
	// 	//     dummy_value);

	// 	if (put_hashmap(hashmap,
	// 	                &random_keys[rand() % (KEYPOOL_SIZE - 1)],
	// 	                dummy_value) == NULL) {
	// 		XFREE(dummy_value, "main : setup");
	// 	}
	// }

	//----------------------------------------------------------- input loop
	for (;;) {
		fputs("\x1b[102m > \x1b[0m", stdout);
		scanf("%s", key);
		// fgets(key, 255, stdin);

		if ((strcmp(key, "exit")) == 0) {
			break;
		}

		if ((strcmp(key, "dump")) == 0) {
			dump_hashmap(hashmap);
			continue;
		}

		if ((strcmp(key, "seed")) == 0) {
			print_xor_seed(hashmap);
			continue;
		}
#ifdef DEBUG_HMAP
		if ((strcmp(key, "stats")) == 0) {
			stats_hashmap(hashmap);
			continue;
		}
#endif /* DEBUG_HMAP */

		if ((strcmp(key, "add")) == 0) {
			printf("todo\n");
			continue;
		}

		hash = hash_tab((unsigned char *)key, hashmap->xor_seed);
		printf(
		    "key        : %s\n"
		    "key        : %s\n"
		    "hash       : %lx\n"
		    "hash_index : %lx\n"
		    "hash_meta  : %x\n",
		    key,
		    (unsigned char *)key,
		    hash,
		    hash_index(hash),
		    hash_meta(hash));
	}

	//-------------------------------------------------------------- cleanup
	hmap_delete_hashmap(hashmap);
	return 0;
}